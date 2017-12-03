#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node * ast = NULL;

// Reverses the order of chained ast nodes
// Due to the bottom up parsing, these nodes are allocated in reverse order
void ast_reorder(node * ast) {
    node * old_chain, * new_chain, * current;
    switch (ast->kind) {
        case SCOPE_NODE:
            new_chain = NULL;
            old_chain = ast->scope.decls;
            while (old_chain != NULL) {
                current = old_chain;
                old_chain = current->decls.next;
                current->decls.next = new_chain;
                new_chain = current;
            }
            ast->scope.decls = new_chain;
            new_chain = NULL;
            old_chain = ast->scope.stmts;
            while (old_chain != NULL) {
                current = old_chain;
                old_chain = current->stmts.next;
                current->stmts.next = new_chain;
                new_chain = current;
            }
            ast->scope.stmts = new_chain;
            break;
        case FUNCTION_NODE:
            new_chain = NULL;
            old_chain = ast->function.args;
            while (old_chain != NULL) {
                current = old_chain;
                old_chain = current->args.next;
                current->args.next = new_chain;
                new_chain = current;
            }
            ast->function.args = new_chain;
            break;
        case CONSTRUCTOR_NODE:
            new_chain = NULL;
            old_chain = ast->constructor.args;
            while (old_chain != NULL) {
                current = old_chain;
                old_chain = current->args.next;
                current->args.next = new_chain;
                new_chain = current;
            }
            ast->constructor.args = new_chain;
            break;
        default: break;
    }
    return;
}

node *ast_allocate(node_kind kind, ...) {
   	va_list args;

    // make the node
    node *new_node = (node *) malloc(sizeof(node));
    memset(new_node, 0, sizeof *new_node);
    new_node->kind = kind;
    new_node->line = yyline;
	new_node->table = cur_table;

    va_start(args, kind); 

    switch(kind) {

    case SCOPE_NODE:
        new_node->scope.decls = va_arg(args, node *);
        new_node->scope.stmts = va_arg(args, node *);
        ast = new_node;
        break;

    case DECLARATIONS_NODE:
        new_node->decls.decl = va_arg(args, node *);
        new_node->decls.next = va_arg(args, node *);
        break;

    case STATEMENTS_NODE:
        new_node->stmts.stmt = va_arg(args, node *);
        new_node->stmts.next = va_arg(args, node *);
        break;

    case UNARY_EXPRESSION_NODE:
        new_node->unary_expr.op = va_arg(args, int);
        new_node->unary_expr.right = va_arg(args, node *);
        break;

    case BINARY_EXPRESSION_NODE:
        new_node->binary_expr.op = va_arg(args, int);
        new_node->binary_expr.left = va_arg(args, node *);
        new_node->binary_expr.right = va_arg(args, node *);
        break;

    case INT_NODE:
        new_node->i_lit = va_arg(args, int);
        break;

    case FLOAT_NODE:
        new_node->f_lit = (float)va_arg(args, double);
        break;

    case BOOL_NODE:
        new_node->b_lit = (bool)va_arg(args, int);
        break;

    case VAR_NODE:
        new_node->variable.id = va_arg(args, char *);
        new_node->variable.is_array = va_arg(args, int);
		new_node->variable.offset = va_arg(args, int);
        break;

    case FUNCTION_NODE:
        new_node->function.func = va_arg(args, int);
        new_node->function.args = va_arg(args, node *);
        break;

    case ARGUMENTS_NODE:
        new_node->args.arg = va_arg(args, node *);
        new_node->args.next = va_arg(args, node *);
        break;

    case CONSTRUCTOR_NODE:
        new_node->constructor.type = va_arg(args, int);
        new_node->constructor.args = va_arg(args, node *);
        break;

    case IF_STATEMENT_NODE:
        new_node->if_stmt.expr = va_arg(args, node *);
        new_node->if_stmt.if_body = va_arg(args, node *);
        new_node->if_stmt.else_body = va_arg(args, node *);
        break; 

    case ASSIGNMENT_NODE:
        new_node->assn.var = va_arg(args, node *);
        new_node->assn.expr = va_arg(args, node *);
        break;

    case DECLARATION_NODE:
		new_node->decl.is_const = va_arg(args, int);
        new_node->decl.type = va_arg(args, int);
        new_node->decl.id = va_arg(args, char *);
        new_node->decl.expr = va_arg(args, node *);
        break;

    default: break;
    }

    va_end(args);

    return new_node;
}

void ast_free(node *ast) {

    if (ast == NULL) {
        return;
    }

    switch (ast->kind) {

    case SCOPE_NODE:
        ast_free(ast->scope.decls);
        ast_free(ast->scope.stmts);
        break;

    case DECLARATIONS_NODE:
        ast_free(ast->decls.decl);
        ast_free(ast->decls.next);
        break;

    case STATEMENTS_NODE:
        ast_free(ast->stmts.stmt);
        ast_free(ast->stmts.next);
        break;

    case UNARY_EXPRESSION_NODE:
        ast_free(ast->unary_expr.right);
        break;

    case BINARY_EXPRESSION_NODE:
        ast_free(ast->binary_expr.left);
        ast_free(ast->binary_expr.right);
        break;

    case VAR_NODE:
        free(ast->variable.id);
        break;

    case FUNCTION_NODE:
        ast_free(ast->function.args);
        break;

    case ARGUMENTS_NODE:
        ast_free(ast->args.arg);
        ast_free(ast->args.next);
        break;

    case CONSTRUCTOR_NODE:
        ast_free(ast->constructor.args);
        break;

    case IF_STATEMENT_NODE:
        ast_free(ast->if_stmt.expr);
        ast_free(ast->if_stmt.if_body);
        ast_free(ast->if_stmt.else_body);
        break;

    case ASSIGNMENT_NODE:
        ast_free(ast->assn.var);
        ast_free(ast->assn.expr);
        break;

    case DECLARATION_NODE:
        ast_free(ast->decl.expr);
        break;

    default:
        break;
    }

    free(ast);
    return;
}

static int indent = 0;
static bool print_inline = false;

static void print_indent() {
    if (print_inline) return;
    int i = indent;
    for (; i > 0; i--) {
        printf("  ");
    }
}

void ast_print(node * ast) {

    if (ast == NULL) {
        return;
    }

    switch (ast->kind) {

    case SCOPE_NODE:
        print_indent(); indent++;
        printf("(SCOPE \n");
        ast_print(ast->scope.decls);
        ast_print(ast->scope.stmts);
        indent--; print_indent();
        printf(")\n");
        break;

    case DECLARATIONS_NODE:
        print_indent(); indent++;
        printf("(DECLARATIONS \n");
        while (ast != NULL) {
            ast_print(ast->decls.decl);
            ast = ast->decls.next;
        }
        indent--; print_indent();
        printf(")\n");
        break;

    case STATEMENTS_NODE:
        print_indent(); indent++;
        printf("(STATEMENTS \n");
        while (ast != NULL) {
            ast_print(ast->stmts.stmt);
            ast = ast->stmts.next;
        }
        indent--; print_indent();
        printf(")\n");
        break;

    case UNARY_EXPRESSION_NODE:
        print_indent(); indent++;
        printf("(UNARY %s %s \n",
            type_as_str(ast->unary_expr.type),
            op_as_str(ast->unary_expr.op));
        ast_print(ast->unary_expr.right);
        indent--; print_indent();
        printf(")\n");
        break;

    case BINARY_EXPRESSION_NODE:
        print_indent(); indent++;
        printf("(BINARY %s %s\n",
            type_as_str(ast->binary_expr.type),
            op_as_str(ast->unary_expr.op));
        ast_print(ast->binary_expr.left);
        ast_print(ast->binary_expr.right);
        indent--; print_indent();
        printf(")\n");
        break;

    case INT_NODE:
        print_indent();
        printf("%d", ast->i_lit);
        printf("\n");
        break;

    case FLOAT_NODE:
        print_indent();
        printf("%f", ast->f_lit);
        printf("\n");
        break;

    case BOOL_NODE:
        print_indent();
        if (ast->b_lit) {
            printf("true");
        } else {
            printf("false");
        }
        printf("\n");
        break;

    case VAR_NODE:
        print_indent();
        const char * footer;
        if (print_inline) {
            footer = "";
        } else {
            footer = "\n";
        }
        if (!ast->variable.is_array) {
            printf("%s%s", ast->variable.id, footer);
        } else {
            Symbol * symbol = table_lookup(ast->variable.id, ast->table, false);
            printf("(index %s %s %d)%s",
                type_as_str(symbol->type),
                symbol->id,
                ast->variable.offset,
                footer);
        }
        break;

    case FUNCTION_NODE:
        print_indent(); indent++;
        const char * func_name;
        if (ast->function.func == DP3) {
            func_name = "DP3";
        } else if (ast->function.func == LIT) {
            func_name = "LIT";
        } else if (ast->function.func == RSQ) {
            func_name = "RSQ";
        } else {
            func_name = "UNK";
        }
        printf("(CALL %s", func_name);
        if (ast->function.args != NULL) {
            printf("\n");
            ast_print(ast->function.args);
            indent--; print_indent();
            printf(")\n");
        } else {
            printf(")");
        }
        break;

    case ARGUMENTS_NODE:
        while (ast != NULL) {
            ast_print(ast->args.arg);
            ast = ast->args.next;
        }
        break;

    case CONSTRUCTOR_NODE:
        print_indent(); indent++;
        printf("(CONSTRUCTOR %s\n", type_as_str(ast->constructor.type));
        ast_print(ast->constructor.args);
        indent--; print_indent();
        printf(")\n");
        break;

    case IF_STATEMENT_NODE:
        print_indent(); indent++;
        printf("(IF\n");
        ast_print(ast->if_stmt.expr);
        ast_print(ast->if_stmt.if_body);
        if (ast->if_stmt.else_body != NULL) {
            ast_print(ast->if_stmt.else_body);
        }
        indent--; print_indent();
        printf(")\n");
        break;

    case ASSIGNMENT_NODE:
        print_indent(); indent++;
        printf("(ASSIGN ");
        if (!ast->assn.var->variable.is_array) {
            Symbol * symbol = table_lookup(ast->assn.var->variable.id, ast->table, false);
            printf("%s ", type_as_str(symbol->type));
        }
        print_inline = true; ast_print(ast->assn.var); print_inline = false;
        printf("\n");
        ast_print(ast->assn.expr);
        indent--; print_indent();
        printf(")\n");
        break;

    case DECLARATION_NODE:
        print_indent();
        printf("(DECLARATION %s %s",
            ast->decl.id,
            type_as_str(ast->decl.type));
        if (ast->decl.expr != NULL) {
            printf("\n");
            indent++;
            ast_print(ast->decl.expr);
            indent--; print_indent();
        }
        printf(")\n");
        break;

	default:
		break;
    }
}
