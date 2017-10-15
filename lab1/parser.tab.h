/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKEN_INT = 258,
     TOKEN_FLOAT = 259,
     TOKEN_BOOL = 260,
     TOKEN_TYPE = 261,
     TOKEN_ID = 262,
     TOKEN_ASSIGN = 263,
     TOKEN_PLUS = 264,
     TOKEN_MINUS = 265,
     TOKEN_MUL = 266,
     TOKEN_DIV = 267,
     TOKEN_EXP = 268,
     TOKEN_NOT = 269,
     TOKEN_AND = 270,
     TOKEN_OR = 271,
     TOKEN_EQ = 272,
     TOKEN_NE = 273,
     TOKEN_GT = 274,
     TOKEN_LT = 275,
     TOKEN_GTE = 276,
     TOKEN_LTE = 277,
     TOKEN_O_SQ_BRKT = 278,
     TOKEN_C_SQ_BRKT = 279,
     TOKEN_O_RD_BRKT = 280,
     TOKEN_C_RD_BRKT = 281,
     TOKEN_O_CURLY = 282,
     TOKEN_C_CURLY = 283,
     TOKEN_COMMA = 284,
     TOKEN_SEMICOLON = 285,
     TOKEN_NEWLINE = 286,
     TOKEN_CONST = 287,
     TOKEN_IF = 288,
     TOKEN_ELSE = 289,
     TOKEN_WHILE = 290
   };
#endif
/* Tokens.  */
#define TOKEN_INT 258
#define TOKEN_FLOAT 259
#define TOKEN_BOOL 260
#define TOKEN_TYPE 261
#define TOKEN_ID 262
#define TOKEN_ASSIGN 263
#define TOKEN_PLUS 264
#define TOKEN_MINUS 265
#define TOKEN_MUL 266
#define TOKEN_DIV 267
#define TOKEN_EXP 268
#define TOKEN_NOT 269
#define TOKEN_AND 270
#define TOKEN_OR 271
#define TOKEN_EQ 272
#define TOKEN_NE 273
#define TOKEN_GT 274
#define TOKEN_LT 275
#define TOKEN_GTE 276
#define TOKEN_LTE 277
#define TOKEN_O_SQ_BRKT 278
#define TOKEN_C_SQ_BRKT 279
#define TOKEN_O_RD_BRKT 280
#define TOKEN_C_RD_BRKT 281
#define TOKEN_O_CURLY 282
#define TOKEN_C_CURLY 283
#define TOKEN_COMMA 284
#define TOKEN_SEMICOLON 285
#define TOKEN_NEWLINE 286
#define TOKEN_CONST 287
#define TOKEN_IF 288
#define TOKEN_ELSE 289
#define TOKEN_WHILE 290




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 57 "parser.y"
{
  int _int;
  float _float;
  bool _bool;
  char * _type;
  char * _id;
}
/* Line 1529 of yacc.c.  */
#line 127 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

