#ifndef _SYMBOL_H
#define _SYMBOL_H

// forward declare
struct node_;
typedef struct node_ node;

typedef enum {
  ANY               = 0, // match none 
  INT               = (1 << 0),
  FLOAT             = (1 << 1), 
  BOOL              = (1 << 2),
  IVEC2 = (1 << 0) | (1 << 3),
  IVEC3 = (1 << 0) | (1 << 4),
  IVEC4 = (1 << 0) | (1 << 5), 
  VEC2  = (1 << 1) | (1 << 6),
  VEC3  = (1 << 1) | (1 << 7),
  VEC4  = (1 << 1) | (1 << 8),
  BVEC2 = (1 << 2) | (1 << 9),
  BVEC3 = (1 << 2) | (1 << 10),
  BVEC4 = (1 << 2) | (1 << 11),
} var_type;


// for semantic checking
enum {
	AND_OP,
	OR_OP,
	NEQ_OP,
	EQ_OP,
	LEQ_OP,
	GEQ_OP 
};

enum {
  DP3 = 0, 
  LIT = 1, 
  RSQ = 2
};


typedef struct Symbol_ {
	int is_const;
	char *id;
	var_type type;
	int line;

} Symbol;

// for simplicity, we set the initial capability of
// a symbol table as 100
#define NUM_SYMBOLS 100
typedef struct Table_ {
	// parent allows nested scopes
	struct Table_ *parent;
	struct Symbol_ *symbols;
	int num_symbols;
} Table;

// the table corresponding the current scope
extern Table *cur_table;

// allocate a new symbol table
Table *table_allocate();

// invoked during building AST
// it inserts a new symbol into the current scope's table
Table *table_insert(node * ast, int is_const, const char* id, var_type type);

// try to find the symbol given the table for local/global lookup
Symbol *table_lookup(const char *id, Table *table, bool is_local);

// returns the name of a type
const char * type_as_str(int type);

// returns the name of an op
const char * op_as_str(int op);

#endif

