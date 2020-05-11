%{

#include "config.h"
#include "ast.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#define _STDLIB_H
AST* parser_result = NULL;
%}

%union {
	AST *val;
	int type;
    const char* str;
}

%token <str> IDENTIFIER CONSTANT STRING_LITERAL
%token <str> SIZEOF
%token <str> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token <str> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token <str> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token <str> XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token <str> TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token <str> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token <str> BOOL COMPLEX IMAGINARY
%token <str> STRUCT UNION ENUM ELLIPSIS

%token <str> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token <str> MS_CDECL MS_STDCALL

%type <val> primary_expression
%type <val> postfix_expression

%type <val> statement compound_statement block_item_list block_item labeled_statement expression_statment iteration_statement jump_statement 
%type <type> assignment_operator
%start translation_unit
%%
// defination part
translation unit 
    : external_declaration
    | translation_unit external_declaration
    ;

external_declaration
    : function_defination
    | declaration
    ;

function_defination
    : declarator declaration_list compound_statement
    | declarator compound_statement
    ;

declaration_specifiers
    : type_specifier
    | type_specifier declaration_specifiers
    | storage_class_specifier declaration_specifiers
    ;

storage_class_specifier
	: TYPEDEF
	| EXTERN
	| STATIC
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

declarator
    : pointer direct_declarator
    | direct_declarator
    ;

pointer
    : '*'
    ;

direct_declarator
    : IDENTIFIER
    : direct_declarator '(' ')'
    | direct_declarator '(' parameter_list ')'
    | direct_declarator '[' ']'
    | direct_declarator '[' constant_expression ']'
    ;

parameter_list
    : parameter_declaration
    | parameter_list ',' parameter_declaration
    ;

parameter_declaration
    :  declaration_specifiers declarator

declaration
    : declaration_specifiers ';'
    | declaration_specifiers init_declarator_list ';'
    ;

declaration_specifiers
    : storage_class_specifier
    | storage_class_specifier declaration_specifiers
    | type_specifier
    | type_specifier declaration_specifiers
    | type_qualifier 
    | type_qualifier declaration_specifiers
    | function_specifier
    | function_specifier declaration_specifiers
    ;

type_qualifier
    : CONST         { $$ = TP_CONST; }
    | VOLATILE      { $$ = TP_VOLATILE; }
    | RESTRICT      { $$ = TP_RESTRICT; }
    ;
	
function_specifier
    : INLINE        { $$ = ATTR_INLINE; }
	| MS_CDECL      { $$ = ATTR_CDECL; }
	| MS_STDCALL    { $$ = ATTR_STDCALL; }
	;

init_declarator_list
    : init_declarator
    | init_declarator_list ',' init_declarator;

init_declarator
	: declarator
	| declarator '=' initializer
	;

initializer
    : assignment_expression
    | '{' initializer_list '}'
    ;

initializer_list
    : initializer
    | initializer_list, initializer
    ;

// statement part

statement
    : labeled_statement    { $$ = $1; }
    | compound_statement   { $$ = $1; }
    | expression_statment  { $$ = $1; }
    | selection_statment   { $$ = $1; }
    | iteration statement  { $$ = $1; }
    | jump_statement       { $$ = $1; }
    ;

compound_statement
    : '{' '}'                         { $$ = make_empty(); }
    | '{' block_item_list '}'         { $$ = make_block($2); }
    ;

block_item_list
    : block_item                      { $$ = $1; }
    | block_item block_item_list      { $$ = ast_append($1, $2); }
    ;

block_item
    : declaration                     { $$ = $1; }
    | statement                       { $$ = $1; }
    ;

labeled_statement
	: IDENTIFIER ':' statement                { $$ = make_label($1, $3); }
	| CASE constant_expression ':' statement  { $$ = make_label_case($2, $4); }
	| DEFAULT ':' statement                   { $$ = make_label_default($3); }
	;

expression_statment
    : ';'                                     { $$ = make_empty(); }
    expression ';'                            { $$ = $1; }
    ;


iteration_statement
	: WHILE '(' expression ')' statement                                         { $$ = make_loop($3, NULL, $5, NULL, LOOP_WHILE); }
	| DO statement WHILE '(' expression ')' ';'                                  { $$ = make_loop($5, NULL, $2, NULL, LOOP_DOWHILE); }
	| FOR '(' expression_statement expression_statement ')' statement            { $$ = make_loop($4, $3, $6, NULL, LOOP_FOR); }
	| FOR '(' expression_statement expression_statement expression ')' statement { $$ = make_loop($4, $3, $7, $5, LOOP_FOR); }
	| FOR '(' declaration expression_statement ')' statement                     { $$ = make_loop($4, $3, $6, NULL, LOOP_FOR); }
	| FOR '(' declaration expression_statement expression ')' statement          { $$ = make_loop($4, $3, $7, $5, LOOP_FOR); }
	;

jump_statement
	: GOTO IDENTIFIER ';'   { $$ = make_jump(JUMP_GOTO, $2, NULL); }
	| CONTINUE ';'          { $$ = make_jump(JUMP_CONTINUE, NULL, NULL); }
	| BREAK ';'             { $$ = make_jump(JUMP_BREAK, NULL, NULL); }
	| RETURN ';'            { $$ = make_jump(JUMP_RET, NULL, NULL); }
	| RETURN expression ';' { $$ = make_jump(JUMP_RET, NULL, $2); }
	;

// expression part

expression
	: assignment_expression
	| expression ',' assignment_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression { $$ = make_binary_expr($2, $1, $2); }
	;

assignment_operator
	: '='
	| MUL_ASSIGN { $$ = OP_ASSIGN_MUL; }
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
    ;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

cast_expression
	: unary_expression
	| '(' c ')' cast_expression
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

postfix_expression
	: primary_expression                     { $$ = $1 }
	| postfix_expression '[' expression ']'  { $$ = make_binary_expr($1, OP_ARRAY_ACCESS, $3) }
	| postfix_expression '(' ')'             {  }
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	;

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')' 
	;

