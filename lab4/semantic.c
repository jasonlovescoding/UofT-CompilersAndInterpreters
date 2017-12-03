#include "semantic.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

void check_scope(node *);
void check_decls(node *);
void check_stmts(node *);
void check_decl(node *);
void check_stmt(node *);
void check_expr(node *, var_type *);

void check_scope(node *cur) {
	assert (cur->kind == SCOPE_NODE);
	check_decls(cur->scope.decls);
	check_stmts(cur->scope.stmts);
}

void check_decls(node *cur) {
	if (cur == NULL) return; 
	assert (cur->kind == DECLARATIONS_NODE);
	check_decl(cur->decls.decl);
	check_decls(cur->decls.next);
}

void check_stmts(node *cur) {
	if (cur == NULL) return;
	assert (cur->kind == STATEMENTS_NODE);
	check_stmt(cur->stmts.stmt);
	check_stmts(cur->stmts.next);
}

void check_stmt(node *cur) {
	if (cur == NULL) return;
	assert((cur->kind & STATEMENT_NODE) || (cur->kind & SCOPE_NODE));	
	
	if (cur->kind == ASSIGNMENT_NODE) {
		Symbol *symbol = table_lookup(cur->assn.var->variable.id, cur->table, false);
		if (symbol == NULL) {
			// refering to an undeclared variable
			fprintf(errorFile, "Semantic Error on line %d: refering to an undeclared variable %s\n", cur->line, cur->assn.var->variable.id);
			errorOccurred = 1;
			return;
		} else if (symbol->is_const) {
			// assignment to a const
			fprintf(errorFile, "Semantic Error on line %d: assign to const variable %s defined at line %d\n", 
				cur->line, symbol->id, symbol->line);
			errorOccurred = 1;
		}
		var_type expr_type;
		check_expr(cur->assn.expr, &expr_type);
		var_type variable_type = symbol->type;
		if (cur->assn.var->variable.is_array) {
			if (variable_type & INT) {
				variable_type = INT;
			} else if (variable_type & FLOAT) {
				variable_type = FLOAT;
			} else if (variable_type & BOOL) {
				variable_type = BOOL;
			} else {
				fprintf(errorFile, "ERROR: unknown array type %d detected\n", variable_type);
			}
			if (expr_type != variable_type && expr_type != ANY) {
				// type dismatch
				fprintf(errorFile, "Semantic Error on line %d: type %s dismatch when declaring variable %s of type %s\n", 
							cur->line, type_as_str(expr_type), symbol->id, type_as_str(variable_type));
				errorOccurred = 1;
			}
		} else if (expr_type != variable_type && expr_type != ANY) {
			// type dismatch
			fprintf(errorFile, "Semantic Error on line %d: type %s dismatch when declaring variable %s of type %s\n", 
							cur->line, type_as_str(expr_type), symbol->id, type_as_str(variable_type));
			errorOccurred = 1;
		}
	} else if (cur->kind == IF_STATEMENT_NODE) {
		var_type type;
		check_expr(cur->if_stmt.expr, &type);
		if (type != BOOL) { 
			// type dismatch
			fprintf(errorFile, "Semantic Error on line %d: if condition is not BOOL\n", cur->line);
			errorOccurred = 1;
		}
		check_stmt(cur->if_stmt.if_body);
		check_stmt(cur->if_stmt.else_body); // NULL is ok	
	} else if (cur->kind == SCOPE_NODE) {
		check_decls(cur->scope.decls);
		check_stmts(cur->scope.stmts);
	} else {
		// unexpected node type detected
		fprintf(errorFile, "Semantic Error on line %d: unexpected kind %d of statement\n", cur->line, cur->kind);
		errorOccurred = 1;
	}
}

void check_constructor(node *cur, var_type *type, int *is_const) {
	assert(cur->kind == CONSTRUCTOR_NODE);
	
	var_type arg_type[4]; // 4 args at most
	int arg_count = 0;
	node *args_node = cur->constructor.args;
	while (args_node) {
			if (arg_count == 4) {
				// do not support over 4 args
				*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: too many arguments\n", cur->line);
				errorOccurred = 1;
				return;
			}
			check_expr(args_node->args.arg, &arg_type[arg_count]);
			args_node = args_node->args.next;
			arg_count++;
	}
	if (arg_count == 0) {
		// do not support empty argument
		*type = ANY;
		fprintf(errorFile, "Semantic Error on line %d: too few arguments\n", cur->line);
		errorOccurred = 1;
		return;	
	}
	if (is_const != NULL) {
		*is_const = 1;
		for (int i = 0; i < arg_count; i++) {
			if (arg_type[i] != INT && arg_type[i] != BOOL && arg_type[i] != FLOAT) {
				*is_const = 0;
				break;
			}
		}
	}
	if (type == NULL) return;
	switch (cur->constructor.type) {
			case (INT):
					if (arg_count != 1) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for INT\n", cur->line);
							errorOccurred = 1;
					} else if (arg_type[0] != INT && arg_type[0] != ANY) {
							// type dismatch
							fprintf(errorFile, "Semantic Error on line %d: argument type %s dismatch for INT\n", 
								cur->line, type_as_str(arg_type[0]));
							*type = ANY;
							errorOccurred = 1;
					} else {
							// success
							*type = INT;	
					}
					return;
			case (FLOAT):
					if (arg_count != 1) {
							// arg num dismatch	
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for FLOAT\n", cur->line);
							errorOccurred = 1;
					} else if (arg_type[0] != FLOAT && arg_type[0] != ANY) {
							// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s dismatch for FLOAT\n", 
								cur->line, type_as_str(arg_type[0]));
							errorOccurred = 1;
					} else {
							// success
							*type = FLOAT;	
					}
					return;
			case (BOOL):
					if (arg_count != 1) {
							// arg num dismatch
							*type = ANY;	
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for BOOL\n", cur->line);
							errorOccurred = 1;
					} else if (arg_type[0] != BOOL && arg_type[0] != ANY) {
							// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s dismatch for BOOL\n", 
								cur->line, type_as_str(arg_type[0]));
							errorOccurred = 1;
					} else {
							// success
							*type = BOOL;	
					}
					return;
			case (IVEC2):
					if (arg_count != 2) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for IVEC2\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != INT && arg_type[0] != ANY) || 
							   (arg_type[1] != INT && arg_type[1] != ANY)) {
							// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s dismatch for IVEC2\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]));
							errorOccurred = 1;
					} else {
							// success
							*type = IVEC2;	
					}
					return;
			case (IVEC3):
					if (arg_count != 3) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for IVEC3\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != INT && arg_type[0] != ANY) || 
							   (arg_type[1] != INT && arg_type[1] != ANY) ||
							   (arg_type[2] != INT && arg_type[2] != ANY)) {
								// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s, type %s dismatch for IVEC3\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]), type_as_str(arg_type[2]));
							errorOccurred = 1;
					} else {
							// success
							*type = IVEC3;	
					}
					return;
			case (IVEC4):
					if (arg_count != 4) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for IVEC4\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != INT && arg_type[0] != ANY) || 
							   (arg_type[1] != INT && arg_type[1] != ANY) ||
							   (arg_type[2] != INT && arg_type[2] != ANY) ||
							   (arg_type[3] != INT && arg_type[3] != ANY)) {
							// type dismatch	
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s, type %s, type %s dismatch for IVEC4\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]), type_as_str(arg_type[2]), type_as_str(arg_type[3]));
							errorOccurred = 1;
					} else {
							// success
							*type = IVEC4;	
					}
					return;
			case (VEC2):
					if (arg_count != 2) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for VEC2\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != FLOAT && arg_type[0] != ANY) || 
							   (arg_type[1] != FLOAT && arg_type[1] != ANY)) {
							// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s dismatch for VEC2\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]));
							errorOccurred = 1;
					} else {
							// success
							*type = VEC2;	
					}
					return;
			case (VEC3):
					if (arg_count != 3) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for VEC3\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != FLOAT && arg_type[0] != ANY) || 
							   (arg_type[1] != FLOAT && arg_type[1] != ANY) ||
							   (arg_type[2] != FLOAT && arg_type[2] != ANY)) {
								// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s, type %s dismatch for VEC3\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]), type_as_str(arg_type[2]));
							errorOccurred = 1;
					} else {
							// success
							*type = VEC3;	
					}
					return;
			case (VEC4):
					if (arg_count != 4) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for VEC4\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != FLOAT && arg_type[0] != ANY) || 
							   (arg_type[1] != FLOAT && arg_type[1] != ANY) ||
							   (arg_type[2] != FLOAT && arg_type[2] != ANY) ||
							   (arg_type[3] != FLOAT && arg_type[3] != ANY)) {
							// type dismatch	
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s, type %s, type %s dismatch for VEC4\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]), type_as_str(arg_type[2]), type_as_str(arg_type[3]));
							errorOccurred = 1;
					} else {
							// success
							*type = VEC4;	
					}
					return;
			case (BVEC2):
					if (arg_count != 2) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for BVEC2\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != BOOL && arg_type[0] != ANY) || 
							   (arg_type[1] != BOOL && arg_type[1] != ANY)) {
							// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s dismatch for BVEC2\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]));
							errorOccurred = 1;
					} else {
							// success
							*type = BVEC2;	
					}
					return;
			case (BVEC3):
					if (arg_count != 3) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for BVEC3\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != BOOL && arg_type[0] != ANY) || 
							   (arg_type[1] != BOOL && arg_type[1] != ANY) ||
							   (arg_type[2] != BOOL && arg_type[2] != ANY)) {
							// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s, type %s dismatch for BVEC3\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]), type_as_str(arg_type[2]));
							errorOccurred = 1;
					} else {
							// success
							*type = BVEC3;	
					}
					return;
			case (BVEC4):
					if (arg_count != 4) {
							// arg num dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for BVEC4\n", cur->line);
							errorOccurred = 1;
					} else if ((arg_type[0] != BOOL && arg_type[0] != ANY) || 
							   (arg_type[1] != BOOL && arg_type[1] != ANY) ||
							   (arg_type[2] != BOOL && arg_type[2] != ANY) ||
							   (arg_type[3] != BOOL && arg_type[3] != ANY)) {
							// type dismatch// type dismatch
							*type = ANY;
							fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s, type %s, type %s dismatch for BVEC4\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]), type_as_str(arg_type[2]), type_as_str(arg_type[3]));
							errorOccurred = 1;
					} else {
							// success
							*type = BVEC4;	
					}
					return;
			default:
					// unexpected constructor detected
					fprintf(errorFile, "Semantic Error on line %d: unexpected type of constructor detected\n", cur->line);
					errorOccurred = 1;
					*type = ANY;
					return;

	}	
}

void check_decl(node *cur) {
	assert (cur->kind == DECLARATION_NODE);
	if (cur->decl.expr) {
		var_type type;	
		check_expr(cur->decl.expr, &type);
		// the declaration comes with an initialization
		Symbol *symbol = table_lookup(cur->decl.id, cur->table, true);
		assert(symbol != NULL);
		if (symbol->is_const) {
			// the symbol represents a constant variable
			if (cur->decl.expr->kind != INT_NODE && 
				cur->decl.expr->kind != BOOL_NODE &&
				cur->decl.expr->kind != FLOAT_NODE) {
				// if the expression is not literal, it can ONLY be from uniform variables/const constructor
				if (cur->decl.expr->kind == VAR_NODE) {
					if (strcmp("gl_Light_Half", cur->decl.expr->variable.id) &&
						strcmp("gl_Light_Ambient", cur->decl.expr->variable.id) &&
						strcmp("gl_Material_Shininess", cur->decl.expr->variable.id) &&
						strcmp("env1", cur->decl.expr->variable.id) &&
						strcmp("env2", cur->decl.expr->variable.id) &&
						strcmp("env3", cur->decl.expr->variable.id) ) {
						
						fprintf(errorFile, "Semantic Error on line %d: trying to assign non-const to variable %s\n", cur->line, symbol->id);
						errorOccurred = 1; // failed checks
					}
				} else if(cur->decl.expr->kind == CONSTRUCTOR_NODE) {		
					int is_const;
					check_constructor(cur->decl.expr, NULL, &is_const);
					if (!is_const) {
						fprintf(errorFile, "Semantic Error on line %d: trying to assign non-const to variable %s\n", cur->line, symbol->id);
						errorOccurred = 1;
					}	
				} else {
					fprintf(errorFile, "Semantic Error on line %d: trying to assign non-const to const variable %s\n", cur->line, symbol->id);
					errorOccurred = 1; 
			 	}
			}
		}
		if (type != symbol->type && type != ANY) {
			// type dismatch
			fprintf(errorFile, "Semantic Error on line %d: type %s dismatch when declaring variable %s of type %s\n", 
							cur->line, type_as_str(type), symbol->id, type_as_str(symbol->type));
			errorOccurred = 1;
		}
	} 
	// if no initialization, or no fail-return, check success
}

void check_function(node *cur, var_type* type) {
	assert(cur->kind == FUNCTION_NODE);
	
	var_type arg_type[2]; // 2 args at most
	int arg_count = 0;
	node *args_node = cur->constructor.args;
	while (args_node) {
			if (arg_count == 2) {
				// do not support over 2 args
				*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: too many arguments for function call\n", cur->line);
				errorOccurred = 1;
				return;
			}
			check_expr(args_node->args.arg, &arg_type[arg_count]);
			args_node = args_node->args.next;
			arg_count++;
	}
	if (arg_count == 0) {
		// do not support empty argument
		*type = ANY;
		fprintf(errorFile, "Semantic Error on line %d: too few arguments for function call\n", cur->line);
		errorOccurred = 1;
		return;	
	}

	switch (cur->function.func) {
		case (LIT):
			if (arg_count != 1) {
				// arg num dismatch
				*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for call to LIT\n", cur->line);
				errorOccurred = 1;	
			} else if (arg_type[0] != VEC4 && arg_type[0] != ANY) {
				// arg type dismatch
				*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: argument type %s dismatch for call to LIT\n", 
									cur->line, type_as_str(arg_type[0]));
				errorOccurred = 1;
			} else {
				// success;
				*type = VEC4;	
			}
			return;
		case (RSQ):
			if (arg_count != 1) {
				// arg num dismatch
				*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for call to RSQ\n", cur->line);
				errorOccurred = 1;	
			} else if (arg_type[0] != FLOAT && arg_type[0] != INT && arg_type[0] != ANY) {
				// arg type dismatch
				*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: argument type %s dismatch for call to RSQ\n", 
								cur->line, type_as_str(arg_type[0]));
				errorOccurred = 1;
			} else {
				// success;
				*type = arg_type[0];	
			}
			return;
		case (DP3):
			if (arg_count != 2) {
				// arg num dismatch
				*type = ANY;
				errorOccurred = 1;	
				fprintf(errorFile, "Semantic Error on line %d: argument number dismatch for call to DP3\n", cur->line);
			} else if ((arg_type[0] == VEC4 || arg_type[0] == ANY) && 
					   (arg_type[1] == VEC4 || arg_type[1] == ANY)) {
				// success
				*type = FLOAT;
			} else if ((arg_type[0] == IVEC4 || arg_type[0] == ANY) && 
					   (arg_type[1] == IVEC4 || arg_type[1] == ANY)) {
				// success
				*type = INT;
			} else if ((arg_type[0] == VEC3 || arg_type[0] == ANY) && 
					   (arg_type[1] == VEC3 || arg_type[1] == ANY)) {
				// success
				*type = FLOAT;
		    } else if ((arg_type[0] == IVEC3 || arg_type[0] == ANY) && 
					   (arg_type[1] == IVEC3 || arg_type[1] == ANY)) {
				// success
				*type = INT;
			} else {
				// type dismatch
				*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: argument type %s, type %s dismatch for call to DP3\n", 
								cur->line, type_as_str(arg_type[0]), type_as_str(arg_type[1]));
				errorOccurred = 1;
			}	
			return;

		default:
			// unexpected function node detected
			fprintf(errorFile, "Semantic Error on line %d: unexpected function type detected\n", cur->line);
			errorOccurred = 1;
			*type = ANY;
			return;
	}
}

void check_unary_expr(node *cur, var_type* type) {
	assert(cur->kind == UNARY_EXPRESSION_NODE);	
	var_type arg_type;
	check_expr(cur->unary_expr.right, &arg_type);
	
	switch(cur->unary_expr.op) {
		case '-':
			if (arg_type & INT || arg_type & FLOAT) {	
				*type = arg_type;	
			} else if (arg_type == ANY) {
				*type = ANY;
			} else {
				// type dismatch				
				fprintf(errorFile, "Semantic Error on line %d: type dismatch for unary op '-'\n", cur->line);
				errorOccurred = 1;
				*type = ANY;
			}
			cur->unary_expr.type = *type;
			return;
		case '!':
			if (arg_type & BOOL) {	
				*type = arg_type;	
			} else if (arg_type == ANY) {
				*type = ANY;
			} else {
				// type dismatch
				fprintf(errorFile, "Semantic Error on line %d: type dismatch for unary op '!'\n", cur->line);
				errorOccurred = 1;
				*type = ANY;
			}
			cur->unary_expr.type = *type;
			return;
		
		default:
			// unexpected unary node detected
			fprintf(errorFile, "Semantic Error on line %d: unexpected unary op %d detected\n", cur->line, cur->unary_expr.op);
			errorOccurred = 1;
			*type = ANY;
			cur->unary_expr.type = *type;
			return;
	}		
}

void check_binary_expr(node *cur, var_type* type) {
	assert(cur->kind == BINARY_EXPRESSION_NODE);
	
	var_type arg_type[2];
	check_expr(cur->binary_expr.left, &arg_type[0]);		
	check_expr(cur->binary_expr.right, &arg_type[1]);
	
	switch(cur->binary_expr.op) {
		case AND_OP:
		case OR_OP:
			if (arg_type[0] == BVEC2 || arg_type[0] == BVEC3 || arg_type[0] == BVEC4) {
				// the first arg is a bool vec
				if (arg_type[1] != arg_type[0] && arg_type[1] != BOOL && arg_type[1] != ANY) {
					// type dismatch
					*type = ANY;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
					errorOccurred = 1;
				} else {
					// success
					*type = arg_type[0];
				}
			} else if (arg_type[0] == BOOL) {
				// the first arg is a bool scalar
				if (arg_type[1] & BOOL) {
					// success
					*type = arg_type[1];
				} else if (arg_type[1] == ANY) {
					*type = BOOL;
				} else {
					// type dismatch
					errorOccurred = 1;
					*type = ANY;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else if (arg_type[0] == ANY) {
				// type not sure for first arg
				if (arg_type[1] & BOOL) {
					*type = arg_type[1];
				} else if (arg_type[1] == ANY) {
					*type = ANY;
				} else {
					// type dismatch
					errorOccurred = 1;
					*type = ANY;
				fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else {
				// type dismatch
				errorOccurred = 1;
				*type = ANY;	
				fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
			}
			cur->binary_expr.type = *type;
			return;
		
		case '<':
		case '>':
		case LEQ_OP:
		case GEQ_OP:
		case EQ_OP:
		case NEQ_OP:
			if (arg_type[0] == INT) {
				// 1st arg is an integer
				if (INT & arg_type[1]) {
					if (arg_type[1] == INT) {
						*type = BOOL;
					} else if (arg_type[1] == IVEC2) {
						*type = BVEC2;
					} else if (arg_type[1] == IVEC3) {
						*type = BVEC3;
					} else if (arg_type[1] == IVEC4) {
						*type = BVEC4;
					} else {
						// type dismatch
						*type = ANY;
						errorOccurred = 1;
						fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
					}	
				} else if (arg_type[1] == ANY) {
					// if the 2nd type is not confirmed, the type is taken as bool
					*type = BOOL;
				} else {
					// type dismatch
					*type = ANY;
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else if (arg_type[0] == FLOAT) {
				// 1st arg is a float
				if (INT & arg_type[1]) {
					if (arg_type[1] == FLOAT) {
						*type = INT;
					} else if (arg_type[1] == VEC2) {
						*type = BVEC2;
					} else if (arg_type[1] == VEC3) {
						*type = BVEC3;
					} else if (arg_type[1] == VEC4) {
						*type = BVEC4;
					} else {
						// type dismatch
						*type = ANY;
						errorOccurred = 1;
						fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
					}	
				} else if (arg_type[1] == ANY) {
					// if the 2nd type is not confirmed, take bool 
					*type = arg_type[0];
				} else {
					// type dismatch
					*type = ANY;
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}	
			} else if (arg_type[0] == IVEC2 ||
					   arg_type[0] == VEC2) {
				// vector comparison must be within the same type or with ANY
				if (arg_type[1] == arg_type[0] || arg_type[1] == ANY) {
					*type = BVEC2;
				} else {
					// type dismatch
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
					*type = ANY;
				} 	
			} else if (arg_type[0] == IVEC3 ||
					   arg_type[0] == VEC3) {
				// vector comparison must be within the same type or with ANY
				if (arg_type[1] == arg_type[0] || arg_type[1] == ANY) {
					*type = BVEC3;
				} else {
					// type dismatch
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
					*type = ANY;
				} 	
			} else if (arg_type[0] == IVEC4 ||
					   arg_type[0] == VEC4) {
				// vector comparison must be within the same type or with ANY
				if (arg_type[1] == arg_type[0] || arg_type[1] == ANY) {
					*type = BVEC4;
				} else {
					// type dismatch
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
					*type = ANY;
				} 	
			} else if (arg_type[0] == ANY) {
				if (arg_type[1] == INT || arg_type[1] == FLOAT) {
					*type = BOOL;
				} else if (arg_type[1] == IVEC2 ||
						   arg_type[1] == VEC2) {
					*type = BVEC2;	
				} else if (arg_type[1] == IVEC3 ||
						   arg_type[1] == VEC3) {
					*type = BVEC3;	
				} else if (arg_type[1] == IVEC4 ||
						   arg_type[1] == VEC4) {
					*type = BVEC4;	
				} else if (arg_type[1] == ANY) {
					*type = ANY;
				} else {
					// type dismatch
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
					*type = ANY;
				}			
			} else {
				// type dismatch
				errorOccurred = 1;	
				fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				*type = ANY;
			}
			cur->binary_expr.type = *type;
			return;

		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
			if (arg_type[0] == INT) {
				if (arg_type[1] & INT) {
					*type = arg_type[1];
				} else if (arg_type[1] == ANY) {
					*type = ANY;
				} else {
					// type dismatch
					*type = ANY;
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else if (arg_type[0] == FLOAT) {
				if (arg_type[1] & FLOAT) {
					*type = arg_type[1];
				} else if (arg_type[1] == ANY) {
					*type = ANY;
				} else {
					// type dismatch
					*type = ANY;
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else if (arg_type[0] == IVEC2 || arg_type[0] == IVEC3 || arg_type[0] == IVEC4) {
				if (arg_type[1] == INT || arg_type[1] == arg_type[0]) {
					*type = arg_type[0];
				} else if (arg_type[1] == ANY) {
					*type = ANY;
				} else {
					// type dismatch
					*type = ANY;
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else if (arg_type[0] == VEC2 || arg_type[0] == VEC3 || arg_type[0] == VEC4) {
				if (arg_type[1] == FLOAT || arg_type[1] == arg_type[0]) {
					*type = arg_type[0];
				} else if (arg_type[1] == ANY) {
					*type = ANY;
				} else {
					// type dismatch
					*type = ANY;
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else if (arg_type[0] == ANY) {
				if ((arg_type[1] & INT)	||
					(arg_type[1] & FLOAT) ||
					(arg_type[1] == ANY)) {
					*type = arg_type[1];
				} else {
					// type dismatch
					*type = ANY;
					errorOccurred = 1;
					fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
				}
			} else {	
				// type dismatch
				*type = ANY;
				errorOccurred = 1;
				fprintf(errorFile, "Semantic Error on line %d: type %s, type %s dismatch for binary op %s\n", cur->line, 
										type_as_str(arg_type[0]), type_as_str(arg_type[1]), op_as_str(cur->binary_expr.op));
			}
			cur->binary_expr.type = *type;
			return;
		
		default:
			// type dismatch
			*type = ANY;
			errorOccurred = 1;
			fprintf(errorFile, "Semantic Error on line %d: unexpected binary op %d detected\n", cur->line, cur->binary_expr.op);
			return;
	}
}

void check_variable(node *cur, var_type *type) {
	assert(cur->kind == VAR_NODE);
	Symbol *symbol = table_lookup(cur->variable.id, cur->table, false);
	if (symbol == NULL) {
		// refering to an undeclared variable
		*type = ANY;
		errorOccurred = 1;
		fprintf(errorFile, "Semantic Error on line %d: referring to an undefined variable %s\n", cur->line, cur->variable.id);
		cur->variable.type = ANY;
		return;
	} else {
		if (cur->variable.is_array) {
			if (symbol->type == IVEC2 ||
				symbol->type == IVEC3 ||
			    symbol->type == IVEC4  ) {
				*type = INT;
			} else if (symbol->type == VEC2 ||
				       symbol->type == VEC3 ||
			           symbol->type == VEC4  ) {
				*type = FLOAT;
			} else if (symbol->type == BVEC2 ||
				       symbol->type == BVEC3 ||
			           symbol->type == BVEC4  ) {
				*type = BOOL;
			} else {
				// type dismatch
				*type = ANY;
				errorOccurred = 1;	
				fprintf(errorFile, "Semantic Error on line %d: indexing an non-array %s\n", cur->line, symbol->id);
			}
		} else {
			*type = symbol->type; 	
		}
		cur->variable.type = *type;
		return;
	}		
}

void check_expr(node *cur, var_type *type) {
	assert(cur->kind & EXPRESSION_NODE);
	switch (cur->kind) {
		case CONSTRUCTOR_NODE:
			check_constructor(cur, type, NULL);
			break;
		case FUNCTION_NODE:
			check_function(cur, type);
			break;
		case UNARY_EXPRESSION_NODE:
			check_unary_expr(cur, type);
			break;
		case BINARY_EXPRESSION_NODE:
			check_binary_expr(cur, type);
			break;
		case BOOL_NODE:
			*type = BOOL;
			break;
		case INT_NODE:
			*type = INT;
			break;
		case FLOAT_NODE:
			*type = FLOAT;
			break;
		case VAR_NODE:
			check_variable(cur, type);
			break;
		
		default:
			// unexpected node type detected
			*type = ANY;
			errorOccurred = 1;
			fprintf(errorFile, "Semantic Error on line %d: unexpected expression type detected\n", cur->line);
			break;
	}	
}

int semantic_check(node *ast) { 
	check_scope(ast);
	return !errorOccurred; 
}
