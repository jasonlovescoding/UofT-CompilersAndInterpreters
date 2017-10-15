%{
/***********************************************************************
 * --YOUR GROUP INFO SHOULD GO HERE--
 * 
 *   Interface to the parser module for CSC467 course project.
 * 
 *   Phase 2: Implement context free grammar for source language, and
 *            parse tracing functionality.
 *   Phase 3: Construct the AST for the source language program.
 ***********************************************************************/

/***********************************************************************
 *  C Definitions and external declarations for this module.
 *
 *  Phase 3: Include ast.h if needed, and declarations for other global or
 *           external vars, functions etc. as needed.
 ***********************************************************************/

#include <string.h>
#include "common.h"
//#include "ast.h"
//#include "symbol.h"
//#include "semantic.h"
#define YYERROR_VERBOSE
#define yTRACE(x)    { if (traceParser) fprintf(traceFile, "%s\n", x); }

void yyerror(const char* s);    /* what to do in case of error            */
int yylex();              /* procedure for calling lexical analyzer */
extern int yyline;        /* variable holding current line number   */

%}

/***********************************************************************
 *  Yacc/Bison declarations.
 *  Phase 2:
 *    1. Add precedence declarations for operators (after %start declaration)
 *    2. If necessary, add %type declarations for some nonterminals
 *  Phase 3:
 *    1. Add fields to the union below to facilitate the construction of the
 *       AST (the two existing fields allow the lexical analyzer to pass back
 *       semantic info, so they shouldn't be touched).
 *    2. Add <type> modifiers to appropriate %token declarations (using the
 *       fields of the union) so that semantic information can by passed back
 *       by the scanner.
 *    3. Make the %type declarations for the language non-terminals, utilizing
 *       the fields of the union as well.
 ***********************************************************************/

%{
#define YYDEBUG 1
%}


// TODO:Modify me to add more data types
// Can access me from flex useing yyval

%union {
  int _int;
  float _float;
  bool _bool;
  char * _type;
  char * _id;
}

// Literals and variables
%token TOKEN_INT
%token TOKEN_FLOAT
%token TOKEN_BOOL
%token TOKEN_TYPE
%token TOKEN_ID

// Operators
%token TOKEN_ASSIGN
%token TOKEN_PLUS
%token TOKEN_MINUS
%token TOKEN_MUL
%token TOKEN_DIV
%token TOKEN_EXP
%token TOKEN_NOT
%token TOKEN_AND
%token TOKEN_OR
%token TOKEN_EQ
%token TOKEN_NE
%token TOKEN_GT
%token TOKEN_LT
%token TOKEN_GTE
%token TOKEN_LTE

// Punctuation
%token TOKEN_O_SQ_BRKT
%token TOKEN_C_SQ_BRKT
%token TOKEN_O_RD_BRKT
%token TOKEN_C_RD_BRKT
%token TOKEN_O_CURLY
%token TOKEN_C_CURLY
%token TOKEN_COMMA
%token TOKEN_SEMICOLON
%token TOKEN_NEWLINE

// Keywords
%token TOKEN_CONST
%token TOKEN_IF
%token TOKEN_ELSE
%token TOKEN_WHILE

%start program

%%

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 *  Phase 3:
 *    1. Add code to rules for construction of AST.
 ***********************************************************************/
program
  :   tokens       
  ;

tokens
  :  tokens token  
  |      
  ;

token
  : TOKEN_INT
  | TOKEN_FLOAT
  | TOKEN_BOOL
  | TOKEN_TYPE
  | TOKEN_ID
  | TOKEN_ASSIGN
  | TOKEN_PLUS
  | TOKEN_MINUS
  | TOKEN_MUL
  | TOKEN_DIV
  | TOKEN_EXP
  | TOKEN_NOT
  | TOKEN_AND
  | TOKEN_OR
  | TOKEN_EQ
  | TOKEN_NE
  | TOKEN_GT
  | TOKEN_LT
  | TOKEN_GTE
  | TOKEN_LTE
  | TOKEN_O_SQ_BRKT
  | TOKEN_C_SQ_BRKT
  | TOKEN_O_RD_BRKT
  | TOKEN_C_RD_BRKT
  | TOKEN_O_CURLY
  | TOKEN_C_CURLY
  | TOKEN_COMMA
  | TOKEN_SEMICOLON
  | TOKEN_NEWLINE
  | TOKEN_CONST
  | TOKEN_IF
  | TOKEN_ELSE
  | TOKEN_WHILE
  ;


%%

/***********************************************************************ol
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(const char* s) {
  if (errorOccurred)
    return;    /* Error has already been reported by scanner */
  else
    errorOccurred = 1;
        
  fprintf(errorFile, "\nPARSER ERROR, LINE %d",yyline);
  if (strcmp(s, "parse error")) {
    if (strncmp(s, "parse error, ", 13))
      fprintf(errorFile, ": %s\n", s);
    else
      fprintf(errorFile, ": %s\n", s+13);
  } else
    fprintf(errorFile, ": Reading token %s\n", yytname[YYTRANSLATE(yychar)]);
}

