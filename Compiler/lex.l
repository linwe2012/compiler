%{



#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

#include "parser.h"


%}

%option noyywrap

oct_int		0[0-7]*
dec_int		[1-9][0-9]*
hex_int		0[xX][0-9a-fA-F]+

identifier	[a-zA-Z_][a-zA-Z0-9_]*
whitespace	[[:space:]]+
float		{dec_int}"."[0-9]*

%%

"auto"			{ return(AUTO); }
"void"          { return(VOID); }
"char"          { return(CHAR); }
"short"         { return(SHORT); }
"int"           { return(INT); }
"long"          { return(LONG); }
"float"         { return(FLOAT); }
"double"        { return(DOUBLE); }

"signed"        { return(SIGNED); }
"unsigned"      { return(UNSIGNED); }

"const"			{ return(CONST); }
"extern"		{ return(EXTERN); }
"inline"		{ return(INLINE); }
"static"		{ return(STATIC); }
"volatile"		{ return(VOLATILE); }

"enum"			{ return(ENUM); }
"struct"		{ return(STRUCT); }

"typedef"		{ return(TYPEDEF); }

"break"			{ return(BREAK); }
"case"			{ return(CASE); }
"continue"		{ return(CONTINUE); }
"default"		{ return(DEFAULT); }
"do"			{ return(DO); }
"else"			{ return(ELSE); }
"for"			{ return(FOR); }
"goto"			{ return(GOTO); }
"if"			{ return(IF); }
"return"        { return RETURN; }
"sizeof"		{ return(SIZEOF); }
"switch"		{ return(SWITCH); }
"union"			{ return(UNION); }
"while"			{ return(WHILE); }

"register"		{ return(REGISTER); }
"restrict"		{ return(RESTRICT); }

{identifier}    { COPY_STR(); return IDENTIFIER; }
{dec_int}       { COPY_STR(); return NUM_INT; }
{float}			{ COPY_STR(); return NUM_FLOAT; }
{float}"f"		{ COPY_STR(); return NUM_FLOAT32; }

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
