#include <stdio.h>
#include "codegen.h"
#include "ast.h"
#include <stdlib.h>
#include <assert.h>
#include "parser.tab.h"
#include <string.h>
#include "symbol.h"
#include <stdlib.h>
#define OUTFILE "frag.txt"

FILE *outfile = fopen(OUTFILE, "w");
int tempCount = 0;
int if_var = -1;

void genAssignmentNode(node*);
void genUnaryNode(node*);
void genBinaryNode(node*);
void genFunctionNode(node*);
void genConstructorNode(node*);
void genIfNode(node*);
void genCode(node*);

/* rename variables */
char* var_name_replace(node *ast, char *name, int *depth) {
	if (!strcmp(name, "gl_FragColor")) {
		return strdup("result.color");
	} else if (!strcmp(name, "gl_FragDepth")) {
		return strdup("result.depth");
	} else if (!strcmp(name, "gl_FragCoord")) {
		return strdup("fragment.position");
	} else if (!strcmp(name, "gl_TexCoord")) {
		return strdup("fragment.texcoord");
	} else if (!strcmp(name, "gl_Color")) {
		return strdup("fragment.color");
	} else if (!strcmp(name, "gl_Secondary")) {
		return strdup("fragment.color.secondary");
	} else if (!strcmp(name, "gl_FogFragCoord")) {
		return strdup("fragment.fogcoord");
	} else if (!strcmp(name, "gl_Light_Half")) {
		return strdup("state.light[0].half");
	} else if (!strcmp(name, "gl_Light_Ambient")) {
		return strdup("state.lightmodel.ambient");
	} else if (!strcmp(name, "gl_Material_Shininess")) {
		return strdup("state.material.shininess");	
	} else if (!strcmp(name, "env1")) {
		return strdup("program.env[1]");
	} else if (!strcmp(name, "env2")) {
		return strdup("program.env[2]");
	} else if (!strcmp(name, "env3")) {
		return strdup("program.env[3]");
	} else {
		int d = *depth;
		Table *table = ast->table;
		while (table && d) {
			if (table_lookup(name, table, true)) {
				break;		
			}	
			table = table->parent;
			d--;		
		}
		char newname[50]; // max id is 32. 50 is enough
		// this variable is defined in its parent scope as well 
		sprintf(newname, "%s__%d", name, d);
		//printf("%s at line%d\n", newname, ast->line);
		return strdup(newname);
	}
}

// dealing with duplicate declared names nested in a scope
char *decl_name_replace(node *ast, char *id, int *depth) {	
	char newname[50]; // max id is 32. 50 is enough
	// this variable is defined in its parent scope as well 
	sprintf(newname, "%s__%d", id, *depth);
	//printf("%s at line%d\n", newname, ast->line);
	return strdup(newname);
}

void gl_rename(node *ast, int *depth) {
    if (ast==NULL) return;
	switch (ast->kind) {
	  case SCOPE_NODE            :   
		(*depth)++;
        gl_rename(ast->scope.decls, depth);
        gl_rename(ast->scope.stmts, depth);
		(*depth)--;
        break; 
	  case UNARY_EXPRESSION_NODE :
	    gl_rename(ast->unary_expr.right, depth);
        break;
      case BINARY_EXPRESSION_NODE:
	    gl_rename(ast->binary_expr.left, depth);
        gl_rename(ast->binary_expr.right, depth);
        break;
	  case VAR_NODE              :
	    ast->variable.id = var_name_replace(ast, ast->variable.id, depth);
        break;
      case FUNCTION_NODE         :
	    gl_rename(ast->function.args, depth);
        break;
      case CONSTRUCTOR_NODE      :
	    gl_rename(ast->constructor.args, depth);
        break;
      case ARGUMENTS_NODE        :
        gl_rename(ast->args.arg, depth);
        gl_rename(ast->args.next, depth);
        break;
      case IF_STATEMENT_NODE     :
	    gl_rename(ast->if_stmt.expr, depth);
        gl_rename(ast->if_stmt.if_body, depth);
        gl_rename(ast->if_stmt.else_body, depth);
        break;
      case ASSIGNMENT_NODE       :
	    gl_rename(ast->assn.var, depth);
        gl_rename(ast->assn.expr, depth);
        break;
      case STATEMENTS_NODE       :
        gl_rename(ast->stmts.stmt, depth);
        gl_rename(ast->stmts.next, depth);
        break;
	  case DECLARATION_NODE      :
	    gl_rename(ast->decl.expr, depth);
		ast->decl.id = decl_name_replace(ast, ast->decl.id, depth);
        break;
      case DECLARATIONS_NODE     :
	    gl_rename(ast->decls.decl, depth);
        gl_rename(ast->decls.next, depth);
		break;
	  case BOOL_NODE:
	  case INT_NODE:
      case FLOAT_NODE:
		break;
      default:
        printf("rename gl vars detect unexpected node kind %x\n", ast->kind);
        assert(0);
    }
}


void generateCode(node *ast) {
	fprintf(outfile, "!!ARBfp1.0\n");
	int depth = 0;
	gl_rename(ast, &depth);
	genCode(ast);
	fprintf(outfile, "END");
}

void genCode(node *ast) {
    if (ast == NULL) {
        return;
    }

    int init[4] = {0};
    int i;
    node * cur;

    switch (ast->kind) {

        case SCOPE_NODE: 
            genCode(ast->scope.decls);
            genCode(ast->scope.stmts);
            break;

        case STATEMENTS_NODE:
            genCode(ast->stmts.stmt);
            genCode(ast->stmts.next);
            break;

        case DECLARATIONS_NODE:
            genCode(ast->decls.decl);
            genCode(ast->decls.next);
            break;

        case UNARY_EXPRESSION_NODE:
            switch (ast->unary_expr.right->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    genCode(ast->unary_expr.right);
                    break;
                default:
                    break;
            }
            genUnaryNode(ast);
            break;

        case BINARY_EXPRESSION_NODE:
            switch (ast->binary_expr.left->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    genCode(ast->binary_expr.left);
                    break;
                default:
                    break;
            }
            switch (ast->binary_expr.right->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    genCode(ast->binary_expr.right);
                    break;
                default:
                    break;
            }
            genBinaryNode(ast);
            break;

        case FUNCTION_NODE:
            genFunctionNode(ast);
            break;

        case CONSTRUCTOR_NODE:
            genConstructorNode(ast);
            break;

        case IF_STATEMENT_NODE:
            genIfNode(ast);
            break;

        case ASSIGNMENT_NODE:
            genAssignmentNode(ast);
            break;

        case DECLARATION_NODE:
            if (ast->decl.is_const) {
                switch (ast->decl.expr->kind) {
                    case CONSTRUCTOR_NODE:
                        cur = ast->decl.expr->constructor.args;
                        for (i = 0; cur != NULL; i++) {
                            switch (cur->args.arg->kind) {
                                case BOOL_NODE:
                                    init[i] = cur->args.arg->b_lit;
                                    break;
                                case INT_NODE:
                                    init[i] = cur->args.arg->i_lit;
                                    break;
                                case FLOAT_NODE:
                                    init[i] = cur->args.arg->f_lit;
                                    break;
                                case UNARY_EXPRESSION_NODE:
                                    switch (cur->args.arg->unary_expr.right->kind) {
                                        case INT_NODE:
                                            init[i] = -cur->args.arg->unary_expr.right->i_lit;
                                            break;
                                        case FLOAT_NODE:
                                            init[i] = -cur->args.arg->unary_expr.right->f_lit;
                                            break;
                                        case BOOL_NODE:
                                            init[i] = !cur->args.arg->unary_expr.right->b_lit;
                                            break;
                                        default:
                                            printf("unary expr as const unexpected arg\n");
                                            assert(0);
                                    }
                                    break; 
                                default:
                                    printf("CONSTRUCTOR with unexpected argument type %x\n", cur->args.arg->kind);
                                    assert(0);
                            }
                            cur = cur->args.next;
                        }
                        fprintf(outfile, "PARAM %s = {%d, %d, %d, %d};\n", ast->decl.id, init[0], init[1], init[2], init[3]);
                        break;
                    case VAR_NODE:
                        fprintf(outfile, "PARAM %s = %s;\n", ast->decl.id, ast->decl.expr->variable.id);
                        break;
                    case INT_NODE:
                        fprintf(outfile, "PARAM %s = {%d, 0, 0, 0};\n", ast->decl.id, ast->decl.expr->i_lit);
                        break;
                    case BOOL_NODE:
                        fprintf(outfile, "PARAM %s = {%d, 0, 0, 0};\n", ast->decl.id, ast->decl.expr->b_lit);
                        break;
                    case FLOAT_NODE:
                        fprintf(outfile, "PARAM %s = {%f, 0, 0, 0};\n", ast->decl.id, ast->decl.expr->f_lit);
                        break;
                    default:
                        printf("decl expr with unexpected type\n");
                        assert(0);
                } 
            } else {
                fprintf(outfile, "TEMP %s;\n", ast->decl.id);   
                if (ast->decl.expr != NULL) {
                    switch (ast->decl.expr->kind) {
                        case CONSTRUCTOR_NODE:
                        case UNARY_EXPRESSION_NODE:
                        case BINARY_EXPRESSION_NODE:
                        case FUNCTION_NODE:
                            genCode(ast->decl.expr);
                            fprintf(outfile, "MOV %s, tempVar%d;\n", ast->decl.id, ast->decl.expr->temp);
                            break;
                        case VAR_NODE:
                            fprintf(outfile, "MOV %s, %s;\n", ast->decl.id, ast->decl.expr->variable.id);
                            break;
                        case INT_NODE:
                            fprintf(outfile, "MOV %s, {%d, 0, 0, 0};\n", ast->decl.id, ast->decl.expr->i_lit);
                            break;
                        case BOOL_NODE:
                            fprintf(outfile, "MOV %s, {%d, 0, 0, 0};\n", ast->decl.id, ast->decl.expr->b_lit);
                            break;
                        case FLOAT_NODE:
                            fprintf(outfile, "MOV %s, {%f, 0, 0, 0};\n", ast->decl.id, ast->decl.expr->f_lit);
                            break;
                        default:
                            printf("decl expr with unexpected type\n");
                            assert(0);
                    }
                }
            }
            break;
        default:
            printf("Unhandled node kind in genCode\n");
            assert(0);
    }
}

void genIfNode(node *ast) {
    // Save the condition of the previous if scope
    int prev_if_var = if_var;

    // Evaluate condition
    if (ast->if_stmt.expr->kind == BOOL_NODE) {
        ast->if_stmt.expr->temp = tempCount; tempCount++;
		fprintf(outfile, "TEMP tempVar%d;\n", ast->if_stmt.expr->temp);
        fprintf(outfile, "MOV tempVar%d, %d;\n", ast->if_stmt.expr->temp, ast->if_stmt.expr->b_lit);
    } else {
        genCode(ast->if_stmt.expr);
    }

    // Evaluate condition for current scope
    if_var = tempCount; tempCount++;
	fprintf(outfile, "TEMP tempVar%d;\n", if_var);
    fprintf(outfile, "SUB tempVar%d, 0, tempVar%d;\n", if_var, ast->if_stmt.expr->temp);
    if (prev_if_var != -1) { // AND condition with condition of outer scope
        fprintf(outfile, "ADD tempVar%d, tempVar%d, tempVar%d;\n", if_var, if_var, prev_if_var);
        fprintf(outfile, "SLT tempVar%d, tempVar%d, -1.5;\n", if_var, if_var);
        fprintf(outfile, "SUB tempVar%d, 0, tempVar%d;\n", if_var, if_var);
    }

    // Generate code for if body
    genCode(ast->if_stmt.if_body);

    // Generate code for else body
    if (ast->if_stmt.else_body != NULL) {
        // Invert if condition
        fprintf(outfile, "SUB tempVar%d, tempVar%d, 1;\n", if_var, ast->if_stmt.expr->temp);
        if (prev_if_var != -1) { // AND condition with condition of outer scope
            fprintf(outfile, "ADD tempVar%d, tempVar%d, tempVar%d;\n", if_var, if_var, prev_if_var);
            fprintf(outfile, "SLT tempVar%d, tempVar%d, -1.5;\n", if_var, if_var);
            fprintf(outfile, "SUB tempVar%d, 0, tempVar%d;\n", if_var, if_var);
        }
        genCode(ast->if_stmt.else_body);
    }

    // Restore condition for previous scope
    if_var = prev_if_var;
}

void genAssignmentNode(node * ast) {
    char index_name[4] = {'x', 'y', 'z', 'w'};

    // Outside an if block
    if (if_var == -1) {
        // Check if dealing with an array with offset
        if (ast->assn.var->variable.offset != -1) {
            switch (ast->assn.expr->kind) {
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                case FUNCTION_NODE:
                    genCode(ast->assn.expr);
                    fprintf(outfile, "MOV %s.%c, tempVar%d.x;\n",
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset],
                        ast->assn.expr->temp);
                    break;
                case VAR_NODE:
                    if (ast->assn.expr->variable.offset != -1) {
                        fprintf(outfile, "MOV %s.%c, %s.%c;\n",
                            ast->assn.var->variable.id,
                            index_name[ast->assn.var->variable.offset],
                            ast->assn.expr->variable.id,
                            index_name[ast->assn.expr->variable.offset]);
                    } else {
                        fprintf(outfile, "MOV %s.%c, %s;\n",
                            ast->assn.var->variable.id,
                            index_name[ast->assn.var->variable.offset],
                            ast->assn.expr->variable.id);
                    }
                    break;
                case INT_NODE:
                    fprintf(outfile, "MOV %s.%c, %d;\n",
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset],
                        ast->assn.expr->i_lit);
                    break;
                case BOOL_NODE:
                    fprintf(outfile, "MOV %s.%c, %d;\n",
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset],
                        ast->assn.expr->b_lit);
                    break;
                case FLOAT_NODE:
                    fprintf(outfile, "MOV %s.%c, %f;\n",
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset],
                        ast->assn.expr->f_lit);
                    break;
                default:
                    printf("assn expr with unexpected type\n");
                    assert(0);
            }
        // Not an array access
        } else {
            switch (ast->assn.expr->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                case FUNCTION_NODE:
                    genCode(ast->assn.expr);
                    fprintf(outfile, "MOV %s, tempVar%d;\n", ast->assn.var->variable.id, ast->assn.expr->temp);
                    break;
                case VAR_NODE:
                    fprintf(outfile, "MOV %s, %s;\n", ast->assn.var->variable.id, ast->assn.expr->variable.id);
                    break;
                case INT_NODE:
                    fprintf(outfile, "MOV %s, %d;\n", ast->assn.var->variable.id, ast->assn.expr->i_lit);
                    break;
                case BOOL_NODE:
                    fprintf(outfile, "MOV %s, %d;\n", ast->assn.var->variable.id, ast->assn.expr->b_lit);
                    break;
                case FLOAT_NODE:
                    fprintf(outfile, "MOV %s, %f;\n", ast->assn.var->variable.id, ast->assn.expr->f_lit);
                    break;
                default:
                    printf("assn expr with unexpected type\n");
                    assert(0);
            }
        }

    // Inside an if block
    } else {
        // Check if dealing with an array with offset
        if (ast->assn.var->variable.offset != -1) {
            switch (ast->assn.expr->kind) {
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                case FUNCTION_NODE:
                    genCode(ast->assn.expr);
                    fprintf(outfile, "CMP %s.%c, tempVar%d, tempVar%d, %s.%c;\n",
                        ast->assn.var->variable.id, 
                        index_name[ast->assn.var->variable.offset],
                        if_var,
                        ast->assn.expr->temp,
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset]);
                    break;
                case VAR_NODE:
                    if (ast->assn.expr->variable.offset != -1) {
                        fprintf(outfile, "CMP %s.%c, tempVar%d, %s, %s.%c;\n",
                            ast->assn.var->variable.id,
                            index_name[ast->assn.var->variable.offset],
                            if_var,
                            ast->assn.expr->variable.id,
                            ast->assn.var->variable.id,
                            index_name[ast->assn.var->variable.offset]);
                    } else {
                        fprintf(outfile, "CMP %s, tempVar%d, %s, %s;\n",
                            ast->assn.var->variable.id,
                            if_var,
                            ast->assn.expr->variable.id,
                            ast->assn.var->variable.id);
                    }
                    break;
                case INT_NODE:
                    fprintf(outfile, "CMP %s.%c, tempVar%d, %d, %s.%c;\n",
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset],
                        if_var,
                        ast->assn.expr->i_lit,
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset]);
                    break;
                case BOOL_NODE:
                    fprintf(outfile, "CMP %s.%c, tempVar%d, %d, %s.%c;\n",
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset],
                        if_var,
                        ast->assn.expr->b_lit,
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset]);
                    break;
                case FLOAT_NODE:
                    fprintf(outfile, "CMP %s.%c, tempVar%d, %f, %s.%c;\n",
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset],
                        if_var,
                        ast->assn.expr->f_lit,
                        ast->assn.var->variable.id,
                        index_name[ast->assn.var->variable.offset]);
                    break;
                default:
                    printf("assn expr with unexpected type\n");
                    assert(0);
            }
        // Not an array access
        } else {
            switch (ast->assn.expr->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                case FUNCTION_NODE:
                    genCode(ast->assn.expr);
                    fprintf(outfile, "CMP %s, tempVar%d, tempVar%d, %s;\n", ast->assn.var->variable.id, if_var, ast->assn.expr->temp, ast->assn.var->variable.id);
                    break;
                case VAR_NODE:
                    fprintf(outfile, "CMP %s, tempVar%d, %s, %s;\n", ast->assn.var->variable.id, if_var, ast->assn.expr->variable.id, ast->assn.var->variable.id);
                    break;
                case INT_NODE:
                    fprintf(outfile, "CMP %s, tempVar%d, {%d, 0, 0, 0}, %s;\n", ast->assn.var->variable.id, if_var, ast->assn.expr->i_lit, ast->assn.var->variable.id);
                    break;
                case BOOL_NODE:
                    fprintf(outfile, "CMP %s, tempVar%d, {%d, 0, 0, 0}, %s;\n", ast->assn.var->variable.id, if_var, ast->assn.expr->b_lit, ast->assn.var->variable.id);
                    break;
                case FLOAT_NODE:
                    fprintf(outfile, "CMP %s, tempVar%d, {%f, 0, 0, 0}, %s;\n", ast->assn.var->variable.id, if_var, ast->assn.expr->f_lit, ast->assn.var->variable.id);
                    break;
                default:
                    printf("assn expr with unexpected type\n");
                    assert(0);
            }
        }
    }
}

void genUnaryNode(node *ast) {  
    int temp = tempCount; ast->temp = temp; tempCount++;
    fprintf(outfile, "TEMP tempVar%d;\n", temp);
    switch (ast->unary_expr.op) {
        case '-':
            switch (ast->unary_expr.right->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    fprintf(outfile, "SUB tempVar%d, {0, 0, 0, 0}, tempVar%d;\n", temp, ast->unary_expr.right->temp);
                    break;
                case VAR_NODE:
                    fprintf(outfile, "SUB tempVar%d, {0, 0, 0, 0}, %s;\n", temp, ast->unary_expr.right->variable.id);
                    break;
                case INT_NODE:
                    fprintf(outfile, "SUB tempVar%d, {0, 0, 0, 0}, {%d, 0, 0, 0};\n", temp, ast->unary_expr.right->i_lit);
                    break;
                case FLOAT_NODE:
                    fprintf(outfile, "SUB tempVar%d, {0, 0, 0, 0}, {%f, 0, 0, 0};\n", temp, ast->unary_expr.right->f_lit);
                    break;
                default:
                    printf("Unary minus with unexpected right\n");
                    assert(0);
            }
            break;
        case '!':
            switch (ast->unary_expr.right->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    fprintf(outfile, "SLT tempVar%d, tempVar%d, {1.0, 1.0, 1.0, 1.0};\n", temp, ast->unary_expr.right->temp);
                    break;
                case VAR_NODE:
                    fprintf(outfile, "SLT tempVar%d, %s, {1.0, 1.0, 1.0, 1.0};\n", temp, ast->unary_expr.right->variable.id);
                    break;
                case BOOL_NODE:
                    fprintf(outfile, "SLT tempVar%d, {%d, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0};\n", temp, ast->unary_expr.right->b_lit);
                    break;
                default:
                    printf("Unary negate with unexpected right\n");
                    assert(0);
           }
    }
}

void genBinaryNode(node *ast) {
    int temp = tempCount; ast->temp = temp; tempCount++;
    fprintf(outfile, "TEMP tempVar%d;\n", temp);
    char left[100], right[100];
    switch (ast->binary_expr.op) {
        case AND:
        case OR:
            switch (ast->binary_expr.left->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    sprintf(left, "tempVar%d", ast->binary_expr.left->temp);
                    break;
                case VAR_NODE:
                    strcpy(left, ast->binary_expr.left->variable.id);
                    break;
                case BOOL_NODE:
                    sprintf(left, "{%d, 0.0, 0.0, 0.0}", ast->binary_expr.left->b_lit);
                    break;
                default:
                    printf("AND expression with unexpected left\n");
                    assert(0);
            }
            switch (ast->binary_expr.right->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    sprintf(right, "tempVar%d", ast->binary_expr.right->temp);
                    break;
                case VAR_NODE:
                    strcpy(right, ast->binary_expr.right->variable.id);
                    break;
                case BOOL_NODE:
                    sprintf(right, "{%d, 0.0, 0.0, 0.0}", ast->binary_expr.right->b_lit);
                    break;
                default:
                    printf("AND expression with unexpected right\n");
                    assert(0);
            }
            fprintf(outfile, "ADD tempVar%d, %s, %s;\n", temp, left, right);
            switch (ast->binary_expr.op) {
                case AND:
                    fprintf(outfile, "SGE tempVar%d, tempVar%d, {2.0, 2.0, 2.0, 2.0};\n", temp, temp);
                    break;
                case OR:
                    fprintf(outfile, "SGE tempVar%d, tempVar%d, {1.0, 1.0, 1.0, 1.0};\n", temp, temp);
                    break;
            }
            break;
        case EQ:
        case NEQ:
        case '>':
        case GEQ:
        case '<':
        case LEQ:
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
            switch (ast->binary_expr.left->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    sprintf(left, "tempVar%d", ast->binary_expr.left->temp);
                    break;
                case VAR_NODE:
                    if (ast->binary_expr.left->variable.offset != -1) {
                        sprintf(left, "tempVar%d", tempCount);
                        tempCount++;
                        fprintf(outfile, "TEMP %s;\n", left);
                        switch (ast->binary_expr.left->variable.offset) {
                            case 0:
                                fprintf(outfile, "DP4 %s, %s, {1, 0, 0, 0};\n", left, ast->binary_expr.left->variable.id);
                                break;
                            case 1:
                                fprintf(outfile, "DP4 %s, %s, {0, 1, 0, 0};\n", left, ast->binary_expr.left->variable.id);
                                break; 
                            case 2:
                                fprintf(outfile, "DP4 %s, %s, {0, 0, 1, 0};\n", left, ast->binary_expr.left->variable.id);
                                break; 
                            case 3:
                                fprintf(outfile, "DP4 %s, %s, {0, 0, 0, 1};\n", left, ast->binary_expr.left->variable.id);
                                break; 
                            default:
                                break;
                        }
                    } else {
                        strcpy(left, ast->binary_expr.left->variable.id);
                    }
                    break;
                case INT_NODE:
                    sprintf(left, "{%d, 0.0, 0.0, 0.0}", ast->binary_expr.left->i_lit);
                    break;
                case FLOAT_NODE:
                    sprintf(left, "{%f, 0.0, 0.0, 0.0}", ast->binary_expr.left->f_lit);
                    break;
                default:
                    printf("binary expression with unexpected left\n");
                    assert(0);
            }
            switch (ast->binary_expr.right->kind) {
                case CONSTRUCTOR_NODE:
                case UNARY_EXPRESSION_NODE:
                case BINARY_EXPRESSION_NODE:
                    sprintf(right, "tempVar%d", ast->binary_expr.right->temp);
                    break;
                case VAR_NODE:
                    if (ast->binary_expr.right->variable.offset != -1) {
                        sprintf(right, "tempVar%d", tempCount);
                        tempCount++;
                        fprintf(outfile, "TEMP %s;\n", right);
                        switch (ast->binary_expr.right->variable.offset) {
                            case 0:
                                fprintf(outfile, "DP4 %s, %s, {1, 0, 0, 0};\n", right, ast->binary_expr.right->variable.id);
                                break;
                            case 1:
                                fprintf(outfile, "DP4 %s, %s, {0, 1, 0, 0};\n", right, ast->binary_expr.right->variable.id);
                                break; 
                            case 2:
                                fprintf(outfile, "DP4 %s, %s, {0, 0, 1, 0};\n", right, ast->binary_expr.right->variable.id);
                                break; 
                            case 3:
                                fprintf(outfile, "DP4 %s, %s, {0, 0, 0, 1};\n", right, ast->binary_expr.right->variable.id);
                                break; 
                            default:
                                break;
                        }
                    } else {
                        strcpy(right, ast->binary_expr.right->variable.id);
                    }
                    break;
                case INT_NODE:
                    sprintf(right, "{%d, 0.0, 0.0, 0.0}", ast->binary_expr.right->i_lit);
                    break;
                case FLOAT_NODE:
                    sprintf(right, "{%f, 0.0, 0.0, 0.0}", ast->binary_expr.right->f_lit);
                    break;
                default:
                    printf("binary expression with unexpected right\n");
                    assert(0);
            }
            int max, min;
            switch (ast->binary_expr.op) {
                case EQ:
                    max = temp;
                    min = tempCount; tempCount++;
                    fprintf(outfile, "MAX tempVar%d, %s, %s;\n", max, left, right);
                    fprintf(outfile, "MIN tempVar%d, %s, %s;\n", min, left, right);
                    fprintf(outfile, "SGE tempVar%d, tempVar%d, tempVar%d;\n", temp, min, max);
                    break;
                case NEQ:
                    max = temp;
                    min = tempCount; tempCount++;
                    fprintf(outfile, "MAX tempVar%d, %s, %s;\n", max, left, right);
                    fprintf(outfile, "MIN tempVar%d, %s, %s;\n", min, left, right);
                    fprintf(outfile, "SLT tempVar%d, tempVar%d, tempVar%d;\n", temp, min, max);
                    break;
                case '>':
                    fprintf(outfile, "SLT tempVar%d, %s, %s;\n", temp, right, left);
                    break;
                case GEQ:
                    fprintf(outfile, "SGE tempVar%d, %s, %s;\n", temp, left, right);
                    break;
                case '<':
                    fprintf(outfile, "SLT tempVar%d, %s, %s;\n", temp, left, right);
                    break;
                case LEQ:
                    fprintf(outfile, "SGE tempVar%d, %s, %s;\n", temp, right, left);
                    break;
                case '+':
                    fprintf(outfile, "ADD tempVar%d, %s, %s;\n", temp, left, right);
                    break;
                case '-':
                    fprintf(outfile, "SUB tempVar%d, %s, %s;\n", temp, left, right);
                    break;
                case '*':
                    fprintf(outfile, "MUL tempVar%d, %s, %s;\n", temp, left, right);
                    break;
                case '/':
                    fprintf(outfile, "RCP tempVar%d, %s;\n", temp, right);
                    fprintf(outfile, "MUL tempVar%d, %s, tempVar%d;\n", temp, left, temp);
                    break;
                case '^':
                    fprintf(outfile, "POW tempVar%d, %s, %s;\n", temp, left, right);
                    break;
            }
            break;
        default:
            break;
    }
    return;
}

void genFunctionNode(node * ast) {
    char * arg0 = (char *)malloc(21);
    char * arg1 = (char *)malloc(21);
    int temp = tempCount; ast->temp = temp; tempCount++;

    fprintf(outfile, "TEMP tempVar%d;\n", temp);
    switch (ast->function.args->args.arg->kind) {
        case CONSTRUCTOR_NODE:
        case UNARY_EXPRESSION_NODE:
        case BINARY_EXPRESSION_NODE:
            sprintf(arg0, "tempVar%d", ast->function.args->args.arg->temp);
            break;
        case VAR_NODE:
            strcpy(arg0, ast->function.args->args.arg->variable.id);
            break;
        case INT_NODE:
            sprintf(arg0, "{%d, 0.0, 0.0, 0.0}", ast->function.args->args.arg->i_lit);
            break;
        case FLOAT_NODE:
            sprintf(arg0, "{%f, 0.0, 0.0, 0.0}", ast->function.args->args.arg->f_lit);
            break;
        default:
            printf("FUNCTION with unexpected argument\n");
            assert(0);
    }

    if (ast->function.func == LIT) {
        fprintf(outfile, "LIT tempVar%d, %s;\n", temp, arg0);
    } else if (ast->function.func == RSQ) {
		// RSQ takes a scalar
		switch (ast->function.args->args.arg->variable.offset) {
			case 0:
        		fprintf(outfile, "RSQ tempVar%d, %s.x;\n", temp, arg0);
				break;
			case 1:
        		fprintf(outfile, "RSQ tempVar%d, %s.y;\n", temp, arg0);
				break;
			case 2:
        		fprintf(outfile, "RSQ tempVar%d, %s.z;\n", temp, arg0);
				break;
			case 3:
        		fprintf(outfile, "RSQ tempVar%d, %s.w;\n", temp, arg0);
				break;
			default:
				printf("Array index invalid\n");
				assert(0);
		} 
    } else if (ast->function.func == DP3) {
        switch (ast->function.args->args.next->args.arg->kind) {
            case CONSTRUCTOR_NODE:
            case UNARY_EXPRESSION_NODE:
            case BINARY_EXPRESSION_NODE:
                sprintf(arg1, "tempVar%d", ast->function.args->args.next->args.arg->temp);
                break;
            case VAR_NODE:
                strcpy(arg1, ast->function.args->args.next->args.arg->variable.id);
                break;
            case INT_NODE:
                sprintf(arg1, "{%d, 0.0, 0.0, 0.0}", ast->function.args->args.next->args.arg->i_lit);
                break;
            case FLOAT_NODE:
                sprintf(arg1, "{%f, 0.0, 0.0, 0.0}", ast->function.args->args.next->args.arg->f_lit);
                break;
            default:
                printf("FUNCTION with unexpected argument\n");
                assert(0);
        }
        fprintf(outfile, "DP3 tempVar%d, %s, %s;\n", temp, arg0, arg1);
    }
    free(arg0);
    free(arg1);
    return;
}

void genConstructorNode(node * ast) {
    int temp = tempCount; ast->temp = temp; tempCount++;
    fprintf(outfile, "TEMP tempVar%d;\n", temp);
    float init[4] = {0.0};
    char * args[4] = {NULL};
    int argcount;
    switch (ast->constructor.type) {
        case IVEC2:
        case BVEC2:
        case VEC2:
            argcount = 2;
            break;
        case IVEC3:
        case BVEC3:
        case VEC3:
            argcount = 3;
            break;
        case IVEC4:
        case BVEC4:
        case VEC4:
            argcount = 4;
            break;
        default:
            printf("CONSTRUCTOR with unexpected argument count\n");
            assert(0);
    }

    node *cur = ast->constructor.args;
    for (int i = 0; i < argcount; i++) {
       switch (cur->args.arg->kind) {
           case UNARY_EXPRESSION_NODE:
           case BINARY_EXPRESSION_NODE:
               genCode(cur->args.arg);
               args[i] = (char *)malloc(11);
               sprintf(args[i], "tempVar%d", cur->args.arg->temp);
               break;
           case VAR_NODE:
               args[i] = strdup(cur->args.arg->variable.id);
               break;
           case BOOL_NODE:
               init[i] = cur->args.arg->b_lit;
               break;
           case INT_NODE:
               init[i] = cur->args.arg->i_lit;
               break;
           case FLOAT_NODE:
               init[i] = cur->args.arg->f_lit;
               break;
           default:
               printf("CONSTRUCTOR with unexpected argument\n");
               assert(0);
      }
      cur = cur->args.next;
   }
    
    switch (ast->constructor.type) {
        case IVEC2:
        case BVEC2:
        case VEC2:
            fprintf(outfile, "MOV tempVar%d, {%f, %f, 0, 0};\n", temp, init[0], init[1]);
            break;
        case IVEC3:
        case BVEC3:
        case VEC3:
            fprintf(outfile, "MOV tempVar%d, {%f, %f, %f, 0};\n", temp, init[0], init[1], init[2]);
            break;
        case IVEC4:
        case BVEC4:
        case VEC4:
            fprintf(outfile, "MOV tempVar%d, {%f, %f, %f, %f};\n", temp, init[0], init[1], init[2], init[3]);
            break;
        default:
            printf("CONSTRUCTOR with unexpected argument count\n");
            assert(0);
    }
    
    
    int temp1 = tempCount; tempCount++;
    fprintf(outfile, "TEMP tempVar%d;\n", temp1);
    for (int i = 0; i < argcount; i++) {
        if (args[i]) {
            fprintf(outfile, "DP4 tempVar%d, %s, {1, 0, 0, 0};\n", temp1, args[i]);
            switch (i) {
                case 0:
                    fprintf(outfile, "CMP tempVar%d, {-1, 0, 0, 0}, tempVar%d, {0, 0, 0, 0};\n", temp1, temp1);
                    break;
                    
                case 1:
                    fprintf(outfile, "CMP tempVar%d, {0, -1, 0, 0}, tempVar%d, {0, 0, 0, 0};\n", temp1, temp1);
                    break; 
                
                case 2:
                    fprintf(outfile, "CMP tempVar%d, {0, 0, -1, 0}, tempVar%d, {0, 0, 0, 0};\n", temp1, temp1);
                    break; 
                
                case 3:
                    fprintf(outfile, "CMP tempVar%d, {0, 0, 0, -1}, tempVar%d, {0, 0, 0, 0};\n", temp1, temp1);
                    break; 
                
            }
            fprintf(outfile, "ADD tempVar%d, tempVar%d, tempVar%d;\n", temp, temp1, temp);
        }
        free(args[i]);
    } 
}
