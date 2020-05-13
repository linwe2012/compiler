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
%token <str> XOR_ASSIGN OR_ASSIGN

%token <str> TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token <str> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID INT64
%token <str> BOOL COMPLEX IMAGINARY
%token <str> STRUCT UNION ENUM ELLIPSIS

%token <str> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token <str> MS_CDECL MS_STDCALL

%type <val> primary_expression
%type <val> postfix_expression

%type <val> statement compound_statement block_item_list block_item labeled_statement expression_statment iteration_statement jump_statement 
%type <type> assignment_operator
%start translation_unit

%type <type> type_qualifier type_qualifier_list function_specifier attribute_specifier
%type <type> storage_class_specifier struct_or_union
%type <val> pointer direct_declarator declarator type_specifier init_declarator_list
%type <val> declaration_specifiers declaration
%type <val> parameter_list parameter_declaration
%type <val> enum_specifier enumerator_list  enumerator

%%
// defination part
translation unit 
    : external_declaration
    | translation_unit external_declaration
    ;

external_declaration
    : function_definition
    | declaration
    ;

/*
function_definition
    : declaration_specifiers declarator declaration_list compound_statement
	| declaration_specifiers declarator compound_statement
	;
*/

function_definition
	: declaration_specifiers declarator compound_statement
	;

// declaration part
declaration
    : declaration_specifiers attribute_specifier  init_declarator_list ';' { $$ = make_declaration($1, $2, $3); }
    | declaration_specifiers init_declarator_list ';'                      { $$ = make_declaration($1, ATTR_NONE, $2); }
    ;

declaration_specifiers
    : type_specifier                                   { $$ = $1; }
    | type_specifier declaration_specifiers            { $$ = make_type_specifier_extend($1, $2, ATTR_NONE); }
    | storage_class_specifier declaration_specifiers   { $$ = make_type_specifier_extend($2, NULL, $1); }
	| storage_class_specifier                          { $$ = make_type_specifier_extend($2, NULL, $1); }
    ;

attribute_specifier
	: MS_CDECL      { $$ = ATTR_CDECL; }
	| MS_STDCALL    { $$ = ATTR_STDCALL; }
	;

init_declarator_list
    : init_declarator                           { $$ = $1; }
    | init_declarator ',' init_declarator_list  { $$ = ast_append($1, $3); }
	;

init_declarator
	: declarator                  { $$ = $1 }
	| declarator '=' initializer  { $$ = make_declarator_with_init($1, $2); }
	;

storage_class_specifier
	: TYPEDEF     { $$ = ATTR_TYPEDEF; }
	| EXTERN      { $$ = ATTR_EXTERN; }
	| STATIC      { $$ = ATTR_STATIC; }
	| AUTO        { $$ = ATTR_AUTO; }
	| REGISTER    { $$ = ATTR_REGISTER; }
	;

type_specifier
	: VOID         { $$ = make_type_specifier(TP_VOID); }
	| CHAR         { $$ = make_type_specifier(TP_INT8); }
	| SHORT        { $$ = make_type_specifier(TP_INT16); }
	| INT          { $$ = make_type_specifier(TP_INT32); }
	| LONG         { $$ = make_type_specifier(TP_INT32); }
	| FLOAT        { $$ = make_type_specifier(TP_FLOAT32); }
	| DOUBLE       { $$ = make_type_specifier(TP_FLOAT64); }
	| INT64        { $$ = make_type_specifier(TP_INT64); }
	| SIGNED       { $$ = make_type_specifier(TP_SIGNED); }
	| UNSIGNED     { $$ = make_type_specifier(TP_UNSIGNED); }
	| struct_or_union_specifier { $$ = $1; }
	| enum_specifier            { $$ = $1; }
	| IDENTIFIER                { $$ = make_type_specifier_from_id($1); }
	;

type_qualifier
    : CONST         { $$ = TP_CONST; }
    | VOLATILE      { $$ = TP_VOLATILE; }
    | RESTRICT      { $$ = TP_RESTRICT; }
    ;

declarator
    : pointer direct_declarator { $$ = make_declarator($1, $2); }
    | direct_declarator         { $$ = $1; }
    ;

direct_declarator
    : IDENTIFIER                     { $$ = makr_init_direct_declarator($1); }
	| '(' declarator ')'             { $$ = $2; }
    | direct_declarator '(' ')'      { $$ = make_extent_direct_declarator($1, TP_FUNC, NULL); }
    | direct_declarator '(' parameter_list ')'      { $$ = make_extent_direct_declarator($1, TP_FUNC, $3); }
    | direct_declarator '[' ']'                     { $$ = make_extent_direct_declarator($1, TP_ARRAY, NULL); }
    | direct_declarator '[' constant_expression ']' { $$ = make_extent_direct_declarator($1, TP_ARRAY, $3); }
    ;

pointer
    : '*'                             { $$ = make_ptr(TP_INCOMPLETE, NULL); }
	| '*' type_qualifier_list         { $$ = make_ptr($2, NULL); }
	| '*' pointer                     { $$ = make_ptr(TP_INCOMPLETE, $2); }
	| '*' type_qualifier_list pointer { $$ = make_ptr($2, $3); }
    ;

parameter_list
    : parameter_declaration                     { $$ = $1; }
    | parameter_declaration ',' parameter_list  { $$ = ast_append($1, $2); }
    ;

type_qualifier_list
    : type_qualifier                     { $$ = $1; }
	| type_qualifier_list type_qualifier { $$ = ast_merge_type_qualifier($1, $2); }

enum_specifier
	: ENUM '{' enumerator_list '}'              { $$ = make_enum_define(NULL, $3); }
	| ENUM IDENTIFIER '{' enumerator_list '}'   { $$ = make_enum_define($2, $4); }
	| ENUM IDENTIFIER                           { $$ = make_enum_define($2, NULL); }
	;

enumerator_list
	: enumerator                         { $$ = $1; }
	| enumerator ',' enumerator_list     { $$ = ast_append($1, $2); }
	;

enumerator
	: IDENTIFIER                         { $$ = make_identifier($1); }
	| IDENTIFIER '=' constant_expression { $$ = make_identifier_with_constant_val($1, $3);  }

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'                { $$ = make_struct_or_union_define($1, NULL, $3);  }
	| struct_or_union IDENTIFIER '{' struct_declaration_list '}'     { $$ = make_struct_or_union_define($1, $2, $4);  }
	| struct_or_union IDENTIFIER                                     { $$ = make_struct_or_union_define($1, $2, NULL);  }
	;

struct_or_union
	: STRUCT         { $$ = TP_STRUCT; }
	| UNION          { $$ = TP_UNION; }
	;

struct_declaration_list
	: struct_declaration                           { $$ = $1; }
	| struct_declaration struct_declaration_list   { $$ = ast_append($1, $2) }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'  
	;

specifier_qualifier_list
	: type_specifier                             { $$ =$1; }
	| type_specifier specifier_qualifier_list    { $$ = ast_merge_specifier_qualifier($2, $1, TP_INCOMPLETE); }
	| type_qualifier                             { $$ = ast_merge_specifier_qualifier(NULL,NULL, $1); }
	| type_qualifier specifier_qualifier_list    { $$ = ast_merge_specifier_qualifier($1, NULL, $1); }
	;

struct_declarator_list
	: struct_declarator                                
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator                            { $$ = $1; }
	// | ':' constant_expression               
	| declarator ':' constant_expression    { $$ = make_declarator_bit_field($1, $3); }
	;

parameter_declaration
    : declaration_specifiers declarator
	| declaration_specifiers abstract_declarator 
	| declaration_specifiers
	;

/* Obsolete-style declarator 
identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;
*/

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expression ']'
	| direct_abstract_declarator '(' parameter_type_list ')'
	| '(' parameter_type_list ')'
	;

initializer
    : assignment_expression
    | '{' initializer_list '}'
	| '{' initializer_list ',' '}'
    ;

initializer_list
    : initializer
    | initializer_list, initializer
    ;
/*
type_name
	: type_specifier
	| type_specifier pointer 
	;
*/

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

function_specifier
    : INLINE        { $$ = ATTR_INLINE; }
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

selection_statement
	: IF '(' expression ')' statement                  { $$ = make_ifelse($3, $5, NULL); }
	| IF '(' expression ')' statement ELSE statement   { $$ = make_ifelse($3, $5, $7); }
	| SWITCH '(' expression ')' statement              { $$ = make_switch($3, $5); }
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

constant_expression
	: conditional_expression
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

