%{

#include "config.h"
#include "ast.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#define _STDLIB_H
AST* parser_result = NULL;
%}


%union {
	AST *val;
	int type;
        const char* str;
}

%type <val> expression expression_list
%type <val> argument argument_list
%type <val> statement statement_list
%type <val> block_statement
%type <val> function_call function_signature function_decalre function_define

%token <str> TYPE
%token <str> RETURN
%token <str> IDENTIFIER
%token <str> NUM_INT NUM_FLOAT NUM_FLOAT32

%%

statement_list
        : statement                 { parser_result = $1; $$ = $1; }
        | statement statement_list  { parser_result = $1; $$ = ast_append($1, $2); }

expression
        : NUM_INT { $$ = make_number_int32($1); }
        | NUM_FLOAT32 { $$ = make_number_float($1, 32); }
        | NUM_FLOAT { $$ = make_number_float($1, 0); }
        | IDENTIFIER { $$ = make_identifier($1); }
        | function_call { $$ = $1; }


argument
        : TYPE IDENTIFIER  { $$ = make_symbol($1, $2);  }

argument_list
        : argument ',' argument_list  { $$ = ast_append($1, $3); }
        | argument                 { $$ = $1; }
        | {}                       { $$ = make_empty(); }

expression_list
        : expression ',' expression_list { $$ = ast_append($1, $3);  }
        | expression                   { $$ = $1; }

statement
        : expression_list ';'         { $$ = $1; }
        | ';'              {$$ = make_empty(); }
        | block_statement  {$$ = $1; }
        | function_decalre {$$ = $1; }
        | function_define  {$$ = $1; }
        | RETURN expression ';' { $$ = make_return($2); }

block_statement
        : '{' statement_list '}' { $$ = make_block($2); }

function_call
        : IDENTIFIER '(' expression_list ')' { $$ = make_function_call($1, $3); }

function_signature: TYPE IDENTIFIER '(' argument_list ')' { $$ =  make_function($1, $2, $4); }

function_decalre: function_signature ';'                    { $$ = $1; }

function_define: function_signature '{' statement_list '}' { $$ = make_function_body($1, $3); }


%%

void parser_set_debug(int i)
{
    yydebug = 1;
}