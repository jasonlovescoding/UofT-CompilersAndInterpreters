%{
/**********************************************************************
 *  Name           Student Number
 *  Yi Qi Cao      999865294
 *  Qianhao Zhang  1004654377
 *
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include "string.h"


#define YY_USER_INIT { yyin = inputFile; }
#define    yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS        [ \t]
LETTER    [a-zA-Z]
BOOL      true|false
INT       [0-9]*
FLOAT     {INT}+\.{INT}*
VEC       [ib]?vec[2-4]
ARITH_OP  [+-*/^]
BOOL_OP   !|&&|\|\|
EQUAL_OP  [=<>!]=|<|>

%%

"/*"        {
              int c1      = 0;
              int c2      = yyinput();
              int curline = yyline;
              for(;;) {
                if (c2 == EOF) {
                  fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: Unmatched /*\n", curline);
                  errorOccurred = TRUE;
                  yyterminate();
                }
                if (c1 == '*' && c2 == '/')
                  break;
                c1 = c2;
                c2 = yyinput();
                if (c1 == '\n' && c2 != EOF)
                  yyline++;
              }
            }

"="  { yTRACE(TOKEN_ASSIGN); return TOKEN_ASSIGN; }
"+"  { yTRACE(TOKEN_PLUS);   return TOKEN_PLUS; }
"-"  { yTRACE(TOKEN_MINUS);  return TOKEN_MINUS; }
"*"  { yTRACE(TOKEN_MUL);    return TOKEN_MUL; }
"/"  { yTRACE(TOKEN_DIV);    return TOKEN_DIV; }
"^"  { yTRACE(TOKEN_EXP);    return TOKEN_EXP; }
"!"  { yTRACE(TOKEN_NOT);    return TOKEN_NOT; }
"&&" { yTRACE(TOKEN_AND);    return TOKEN_AND; }
"||" { yTRACE(TOKEN_OR);     return TOKEN_OR ; }
"==" { yTRACE(TOKEN_EQ);     return TOKEN_EQ ; }
"!=" { yTRACE(TOKEN_NE);     return TOKEN_NE ; }
">"  { yTRACE(TOKEN_GT);     return TOKEN_GT ; }
"<"  { yTRACE(TOKEN_LT);     return TOKEN_LT ; }
">=" { yTRACE(TOKEN_GTE);    return TOKEN_GTE; }
"<=" { yTRACE(TOKEN_LTE);    return TOKEN_LTE; }

"["  { yTRACE(TOKEN_O_SQ_BRKT); return TOKEN_O_SQ_BRKT; }
"]"  { yTRACE(TOKEN_C_SQ_BRKT); return TOKEN_C_SQ_BRKT; }
"("  { yTRACE(TOKEN_O_RD_BRKT); return TOKEN_O_RD_BRKT; }
")"  { yTRACE(TOKEN_C_RD_BRKT); return TOKEN_C_RD_BRKT; }
"{"  { yTRACE(TOKEN_O_CURLY);   return TOKEN_O_CURLY; }
"}"  { yTRACE(TOKEN_C_CURLY);   return TOKEN_C_CURLY; }
","  { yTRACE(TOKEN_COMMA);     return TOKEN_COMMA; }
";"  { yTRACE(TOKEN_SEMICOLON); return TOKEN_SEMICOLON; }
"\n" { yyline++; yTRACE(TOKEN_NEWLINE); return TOKEN_NEWLINE; }

const { yTRACE(TOKEN_CONST); return TOKEN_CONST; }
if    { yTRACE(TOKEN_IF);    return TOKEN_IF; }
else  { yTRACE(TOKEN_ELSE);  return TOKEN_ELSE; }
while { yTRACE(TOKEN_WHILE); return TOKEN_WHILE; }

[+-]?{INT}   {
               yTRACE(TOKEN_INT);
               char buffer[sizeof(int) * 8 + 1];
               if (yytext[0] == '+') {
                 yylval._int = atoi(yytext+1);
                 sprintf(buffer, "%d", yylval._int);
                 if (strcmp(buffer, yytext+1) != 0) {
                     yERROR("Failed to convert int literal");
                 }
               } else if (yytext[0] == '-') {
                 yylval._int = -atoi(yytext+1);
                 sprintf(buffer, "%d", yylval._int);
                 if (strcmp(buffer, yytext) != 0) {
                     yERROR("Failed to convert int literal");
                 }
               } else {
                 yylval._int = atoi(yytext);
                 sprintf(buffer, "%d", yylval._int);
                 if (strcmp(buffer, yytext) != 0) {
                     yERROR("Failed to convert int literal");
                 }
               }
               return TOKEN_INT;
             }

[+-]?{FLOAT} {
               yTRACE(TOKEN_FLOAT);
               if (yytext[0] == '+') {
                 yylval._float = atof(yytext+1);
               } else if (yytext[0] == '-') {
                 yylval._float = -atof(yytext+1);
               } else {
                 yylval._float = atof(yytext);
               }
               return TOKEN_FLOAT;
             }

{BOOL}  {
          yTRACE(TOKEN_BOOL);
          if (yytext[0] == 't') {
            yylval._bool = true;
          } else {
            yylval._bool = false;
          }
          return TOKEN_FLOAT;
        }

int|bool|float|{VEC} {
                       yTRACE(TOKEN_TYPE);
                       yylval._type = yytext;
                       return TOKEN_TYPE;
                     }

[a-zA-Z_][a-zA-Z_0-9]* {
                         yTRACE(TOKEN_ID);
                         yylval._id = yytext;
                         return TOKEN_ID;
                       }

{WS}* {}

.    { yTRACE(-1); yERROR("Unknown token"); }

%%
