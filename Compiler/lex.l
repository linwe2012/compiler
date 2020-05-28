%{



#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

#include "parser.h"

#define COPY_STR()             yylval.str = strcpy((char*)malloc(strlen(yytext)+1), yytext)

%}

%option noyywrap

oct_int		0[0-7]*
dec_int		0|([1-9][0-9]*)
hex_int		0[xX][0-9a-fA-F]+

any_int     {oct_int}|{dec_int}|{hex_int}

identifier	[a-zA-Z_][a-zA-Z0-9_]*
whitespace	[[:space:]]+
float		{dec_int}"."[0-9]*

c_comment	\/\*[^*]*\*+([^/*][^*]*\*+)*\/
cpp_comment	\/\/.*
comment		{c_comment}|{cpp_comment}

%%

"auto"			{ return AUTO; }
"void"          { return VOID; }
"char"          { return CHAR; }
"short"         { return SHORT; }
"int"           { return INT; }
"long"          { return LONG; }
"float"         { return FLOAT; }
"double"        { return DOUBLE; }

"signed"        { return SIGNED; }
"unsigned"      { return UNSIGNED; }

"const"			{ return CONST; }
"extern"		{ return EXTERN; }
"inline"		{ return INLINE; }
"static"		{ return STATIC; }
"volatile"		{ return VOLATILE; }

"enum"			{ return ENUM; }
"struct"		{ return STRUCT; }

"typedef"		{ return TYPEDEF; }

"break"			{ return BREAK; }
"case"			{ return CASE; }
"continue"		{ return CONTINUE; }
"default"		{ return DEFAULT; }
"do"			{ return DO; }
"else"			{ return ELSE; }
"for"			{ return FOR; }
"goto"			{ return GOTO; }
"if"			{ return IF; }
"return"        { return RETURN; }
"sizeof"		{ return SIZEOF; }
"switch"		{ return SWITCH; }
"union"			{ return UNION; }
"while"			{ return WHILE; }

"register"		{ return REGISTER; }
"restrict"		{ return RESTRICT; }


"__cdecl"        { return MS_CDECL; }  /*  Microsoft extension */
"__stdcall"      { return MS_STDCALL; }
"__inline"       { return INLINE; }
"__int8"         { return CHAR; }
"__int16"        { return SHORT; }
"__int32"        { return INT; }
"__int64"        { return INT64; }

{identifier}    	{ COPY_STR(); return IDENTIFIER; }
\"(\\.|[^\\"\n])*\"	{ COPY_STR(); return STRING_LITERAL; }
{any_int}       	{ COPY_STR(); return NUM_INT; }
{float}				{ COPY_STR(); return NUM_FLOAT32; }
{float}"f"			{ COPY_STR(); return NUM_FLOAT64; }

{whitespace}    {}
{comment}    	{}

"..."			{ return ELLIPSIS; }
">>="			{ return RIGHT_ASSIGN; }
"<<="			{ return LEFT_ASSIGN; }
"+="			{ return ADD_ASSIGN; }
"-="			{ return SUB_ASSIGN; }
"*="			{ return MUL_ASSIGN; }
"/="			{ return DIV_ASSIGN; }
"%="			{ return MOD_ASSIGN; }
"&="			{ return AND_ASSIGN; }
"^="			{ return XOR_ASSIGN; }
"|="			{ return OR_ASSIGN; }
">>"			{ return RIGHT_OP; }
"<<"			{ return LEFT_OP; }
"++"			{ return INC_OP; }
"--"			{ return DEC_OP; }
"->"			{ return PTR_OP; }
"&&"			{ return AND_OP; }
"||"			{ return OR_OP; }
"<="			{ return LE_OP; }
">="			{ return GE_OP; }
"=="			{ return EQ_OP; }
"!="			{ return NE_OP; }

.               { return yytext[0]; }

%%
