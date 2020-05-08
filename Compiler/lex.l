%{



#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

#include "parser.h"

#define COPY_STR()              yylval.str = strcpy((char*)malloc(strlen(yytext)+1), yytext)
#define RET_TYPE()				return(TYPE)


%}

%option noyywrap

integer [0-9]*
identifier [a-zA-Z_][a-zA-Z0-9_]*
whitespace [[:space:]]+
floatpoint {integer}"."{integer}

%%

"auto"			{ }
"void"          { COPY_STR(); RET_TYPE(); }
"char"          { COPY_STR(); RET_TYPE(); }
"short"         { COPY_STR(); RET_TYPE(); }
"int"           { COPY_STR(); RET_TYPE(); }
"long"          { COPY_STR(); RET_TYPE(); }
"float"         { COPY_STR(); RET_TYPE(); }
"double"        { COPY_STR(); RET_TYPE(); }

"signed"        { /* TODO */ }
"unsigned"      { /* TODO */ }

"const"			{ }
"extern"		{ }
"inline"		{ }
"static"		{ }
"volatile"		{ }

"enum"			{ }
"struct"		{ }

"typedef"		{ }

"break"			{ }
"case"			{ }
"continue"		{ }
"default"		{ }
"do"			{ }
"else"			{ }
"for"			{ }
"goto"			{ }
"if"			{ }
"return"        { return RETURN; }
"sizeof"		{ }
"switch"		{ }
"union"			{ }
"while"			{ }

"register"		{ }
"restrict"		{ }

{identifier}    { COPY_STR(); return IDENTIFIER; }
{integer}       { COPY_STR(); return NUM_INT; }
{floatpoint}    { COPY_STR(); return NUM_FLOAT; }
{floatpoint}"f" { COPY_STR(); return NUM_FLOAT32; }

/* TODO: 八进制 十六进制 科学计数法 */

{whitespace}    /* do nothing */

"//".*          {  }
"/*"			{ comment(); }

">>="			{ return(RIGHT_ASSIGN); }
"<<="			{ return(LEFT_ASSIGN); }
"+="			{ return(ADD_ASSIGN); }
"-="			{ return(SUB_ASSIGN); }
"*="			{ return(MUL_ASSIGN); }
"/="			{ return(DIV_ASSIGN); }
"%="			{ return(MOD_ASSIGN); }
"&="			{ return(AND_ASSIGN); }
"^="			{ return(XOR_ASSIGN); }
"|="			{ return(OR_ASSIGN); }
">>"			{ return(RIGHT_OP); }
"<<"			{ return(LEFT_OP); }
"++"			{ return(INC_OP); }
"--"			{ return(DEC_OP); }
"->"			{ return(PTR_OP); }
"&&"			{ return(AND_OP); }
"||"			{ return(OR_OP); }
"<="			{ return(LE_OP); }
">="			{ return(GE_OP); }
"=="			{ return(EQ_OP); }
"!="			{ return(NE_OP); }

.               { return yytext[0]; }

%%

void comment(void) {
	char c, prev = 0;
  
	while ((c = input()) != 0) {
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	error("unterminated comment");
}
