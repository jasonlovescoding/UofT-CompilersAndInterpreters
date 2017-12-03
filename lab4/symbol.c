#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "common.h"
#include "symbol.h"

Table *table_allocate() {
	Table *table = (Table *) malloc(sizeof(Table));
	if (table == NULL) return NULL;
	table->parent = cur_table;
	table->symbols = (Symbol *)malloc(NUM_SYMBOLS * sizeof(Symbol));	
	table->num_symbols = 0;
	return table;
}

Symbol *table_lookup(const char *id, Table *table, bool is_local) {
	if (table == NULL) {
		return NULL;
	}
	for (int i = 0; i < table->num_symbols; i++) {
			if (!strcmp(id, table->symbols[i].id)) {
					return &(table->symbols[i]);
			}
	}

	if (is_local) {	
		return NULL;
	} else {
		// if global lookup, look up the parent table
		return table_lookup(id, table->parent, is_local);
	}	
}

Table *table_insert(node * ast, int is_const, const char *id, var_type type) {
	Symbol *symbol = NULL;	
	if ((symbol = table_lookup(id, cur_table, 1)) != NULL) {
		if (!strcmp(id, "gl_FragColor") ||
            !strcmp(id, "gl_FragDepth") ||
            !strcmp(id, "gl_FragCoord") ||
			!strcmp(id, "gl_TexCoord")  ||
			!strcmp(id, "gl_Color")     || 
			!strcmp(id, "gl_Secondary") ||
			!strcmp(id, "gl_FogFragCoord")||
			!strcmp(id, "gl_Light_Half")  ||
			!strcmp(id, "gl_Light_Ambient") || 
			!strcmp(id, "gl_Material_Shininess") ||
			!strcmp(id, "env1") ||
			!strcmp(id, "env2") ||
			!strcmp(id, "env3")
		   ) {
			fprintf(stdout, "Semantic Error on line %d: re-defining variable %s which is a predefined variable of the language.\n", 
			ast->line, id);
		} else {
			fprintf(stdout, "Semantic Error on line %d: re-defining variable %s which was defined at line %d\n", 
			ast->line, id, symbol->line);
		} 
        errorOccurred = 1;
		return NULL;
	}
	cur_table->num_symbols++;
	int num_symbols = cur_table->num_symbols;	
	if (num_symbols > NUM_SYMBOLS) {
		cur_table->symbols = (Symbol *)realloc(cur_table->symbols,
											   num_symbols);
	}
	cur_table->symbols[num_symbols - 1].is_const = is_const;
	cur_table->symbols[num_symbols - 1].id = strdup(id); 
	cur_table->symbols[num_symbols - 1].type = type;
	cur_table->symbols[num_symbols - 1].line = yyline;
	return cur_table;
}

const char * type_as_str(int type) {
    switch (type) {
        case ANY:
            return "any";
        case INT:
            return "int";
        case FLOAT:
            return "float";
        case BOOL:
            return "bool";
        case IVEC2:
            return "ivec2";
        case IVEC3:
            return "ivec3";
        case IVEC4:
            return "ivec4";
        case VEC2:
            return "vec2";
        case VEC3:
            return "vec3";
        case VEC4:
            return "vec4";
        case BVEC2:
            return "bvec2";
        case BVEC3:
            return "bvec3";
        case BVEC4:
            return "bvec4";
        default:
            return "invalid";
    }
}

const char * op_as_str(int op) {
    switch (op) {
        case '-':
            return "-";
        case '!':
            return "!";
        case AND_OP:
            return "&&";
        case OR_OP:
            return "||";
        case EQ_OP:
            return "==";
        case NEQ_OP:
            return "!=";
        case '<':
            return "<";
        case LEQ_OP:
            return "<=";
        case '>':
            return ">";
        case GEQ_OP:
            return ">=";
        case '+':
            return "+";
        case '*':
            return "*";
        case '/':
            return "/";
        case '^':
            return "^";
        default:
            return "invalid_op";
    }
}

