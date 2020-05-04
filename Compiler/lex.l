%{



#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

#include "parser.h"

#define COPY_STR()             yylval.str = strcpy((char*)malloc(strlen(yytext)+1), yytext)
#define RET_TYPE()				return(TYPE)
// [[:space:]]+     /* sem ação */ ["\n"" "]


%}

%option noyywrap

integer [0-9]*
identifier [a-zA-Z_][a-zA-Z0-9_]*
whitespace [[:space:]]+
floatpoint {integer}"."{integer}

%%

"void" { COPY_STR(); RET_TYPE(); }
"int" { COPY_STR(); RET_TYPE(); }
"double" {COPY_STR(); RET_TYPE();}
"float" {COPY_STR(); RET_TYPE();}
"return" { return RETURN; }
{identifier} {COPY_STR(); return IDENTIFIER; }
{integer}    {COPY_STR(); return NUM_INT; }
{floatpoint} { COPY_STR(); return NUM_FLOAT; }
{floatpoint}"f" { COPY_STR(); return NUM_FLOAT32; }
{whitespace} /* do nothing */
"//".*  {  }
. { return yytext[0]; }

%%
