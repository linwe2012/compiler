%{
#include "yacc-error.h"

#define YY_INPUT(buf,result,max_size)  {\
    result = GetNextChar(buf, max_size); \
    if (  result <= 0  ) \
      result = YY_NULL; \
    }

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

#include "parser.h"

#define COPY_STR()             yylval.str = strcpy((char*)malloc(strlen(yytext)+1), yytext)
#define PREPARE()              BeginToken(yytext)
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

"auto"			{ PREPARE(); return AUTO; }
"void"          { PREPARE(); return VOID; }
"char"          { PREPARE(); return CHAR; }
"short"         { PREPARE(); return SHORT; }
"int"           { PREPARE(); return INT; }
"long"          { PREPARE(); return LONG; }
"float"         { PREPARE(); return FLOAT; }
"double"        { PREPARE(); return DOUBLE; }

"signed"        { PREPARE(); return SIGNED; }
"unsigned"      { PREPARE(); return UNSIGNED; }

"const"			{ PREPARE(); return CONST; }
"extern"		{ PREPARE(); return EXTERN; }
"inline"		{ PREPARE(); return INLINE; }
"static"		{ PREPARE(); return STATIC; }
"volatile"		{ PREPARE(); return VOLATILE; }

"enum"			{ PREPARE(); return ENUM; }
"struct"		{ PREPARE(); return STRUCT; }

"typedef"		{ PREPARE(); return TYPEDEF; }

"break"			{ PREPARE(); return BREAK; }
"case"			{ PREPARE(); return CASE; }
"continue"		{ PREPARE(); return CONTINUE; }
"default"		{ PREPARE(); return DEFAULT; }
"do"			{ PREPARE(); return DO; }
"else"			{ PREPARE(); return ELSE; }
"for"			{ PREPARE(); return FOR; }
"goto"			{ PREPARE(); return GOTO; }
"if"			{ PREPARE(); return IF; }
"return"        { PREPARE(); return RETURN; }
"sizeof"		{ PREPARE(); return SIZEOF; }
"switch"		{ PREPARE(); return SWITCH; }
"union"			{ PREPARE(); return UNION; }
"while"			{ PREPARE(); return WHILE; }

"register"		{ PREPARE(); return REGISTER; }
"restrict"		{ PREPARE(); return RESTRICT; }


"__cdecl"        { PREPARE(); return MS_CDECL; }  /*  Microsoft extension */
"__stdcall"      { PREPARE(); return MS_STDCALL; }
"__fastcall"     { PREPARE(); return MS_FASTCALL; }
"__inline"       { PREPARE(); return INLINE; }
"__int8"         { PREPARE(); return CHAR; }
"__int16"        { PREPARE(); return SHORT; }
"__int32"        { PREPARE(); return INT; }
"__int64"        { PREPARE(); return INT64; }

{identifier}    	{ PREPARE(); COPY_STR(); return IDENTIFIER; }
\"(\\.|[^\\"\n])*\"	{ PREPARE(); COPY_STR(); return STRING_LITERAL; }
\'.\'               { PREPARE(); COPY_STR(); return CHAR_LITERAL; }
{any_int}       	{ PREPARE(); COPY_STR(); return NUM_INT; }
{float}				{ PREPARE(); COPY_STR(); return NUM_FLOAT32; }
{float}"f"			{ PREPARE(); COPY_STR(); return NUM_FLOAT64; }

{whitespace}    {}
{comment}    	{}

"..."			{ PREPARE(); return ELLIPSIS; }
">>="			{ PREPARE(); return RIGHT_ASSIGN; }
"<<="			{ PREPARE(); return LEFT_ASSIGN; }
"+="			{ PREPARE(); return ADD_ASSIGN; }
"-="			{ PREPARE(); return SUB_ASSIGN; }
"*="			{ PREPARE(); return MUL_ASSIGN; }
"/="			{ PREPARE(); return DIV_ASSIGN; }
"%="			{ PREPARE(); return MOD_ASSIGN; }
"&="			{ PREPARE(); return AND_ASSIGN; }
"^="			{ PREPARE(); return XOR_ASSIGN; }
"|="			{ PREPARE(); return OR_ASSIGN; }
">>"			{ PREPARE(); return RIGHT_OP; }
"<<"			{ PREPARE(); return LEFT_OP; }
"++"			{ PREPARE(); return INC_OP; }
"--"			{ PREPARE(); return DEC_OP; }
"->"			{ PREPARE(); return PTR_OP; }
"&&"			{ PREPARE(); return AND_OP; }
"||"			{ PREPARE(); return OR_OP; }
"<="			{ PREPARE(); return LE_OP; }
">="			{ PREPARE(); return GE_OP; }
"=="			{ PREPARE(); return EQ_OP; }
"!="			{ PREPARE(); return NE_OP; }

.               { PREPARE(); return yytext[0]; }

%%
