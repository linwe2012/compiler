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
%type <val> expression assignment_expression constant_expression conditional_expression 
%type <val> and_expression  logical_and_expression logical_or_expression exclusive_or_expression inclusive_or_expression
%type <val> equality_expression relational_expression shift_expression additive_expression
%type <val> multiplicative_expression cast_expression unary_expression 

%type <type> assignment_operator unary_operator 

%type <val> statement compound_statement block_item_list block_item selection_statement labeled_statement expression_statement iteration_statement jump_statement 

%start translation_unit

%type <type> type_name type_qualifier type_qualifier_list attribute_specifier
%type <type> storage_class_specifier struct_or_union
%type <val> struct_or_union_specifier struct_declaration_list struct_declaration struct_declarator
%type <val> specifier_qualifier_list
%type <val> pointer direct_declarator declarator type_specifier init_declarator_list init_declarator 
%type <val> initializer initializer_list
%type <val> declaration_specifiers declaration
%type <val> parameter_list parameter_declaration
%type <val> enum_specifier enumerator_list  enumerator
%type <val> abstract_declarator direct_abstract_declarator

%%
// defination part
translation_unit 
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
    : declaration_specifiers attribute_specifier init_declarator_list ';' { $$ = make_declaration($1, $2, $3); }
    | declaration_specifiers init_declarator_list ';'                      { $$ = make_declaration($1, ATTR_NONE, $2); }
    ;

declaration_specifiers
    : type_specifier                                   { $$ = $1; }
    | type_specifier declaration_specifiers            { $$ = make_type_specifier_extend($1, $2, ATTR_NONE); }
    | storage_class_specifier declaration_specifiers   { $$ = make_type_specifier_extend($2, NULL, $1); }
	| storage_class_specifier                          { $$ = make_type_specifier_extend(NULL, NULL, $1); }
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
	| declarator '=' initializer  { $$ = make_declarator_with_init($1, $3); }
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
	| LONG         { $$ = make_type_specifier(TP_INT32 | TP_LONG_FLAG); }
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
    | parameter_declaration ',' parameter_list  { $$ = ast_append($1, $3); }
    ;

type_qualifier_list
    : type_qualifier                     { $$ = $1; }
	| type_qualifier_list type_qualifier { $$ = ast_merge_type_qualifier($1, $2); }

declaration
    : declaration_specifiers init_declarator_list ';'
    ;
enum_specifier
	: ENUM '{' enumerator_list '}'              { $$ = make_enum_define(NULL, $3); }
	| ENUM IDENTIFIER '{' enumerator_list '}'   { $$ = make_enum_define($2, $4); }
	| ENUM IDENTIFIER                           { $$ = make_enum_define($2, NULL); }
	;

enumerator_list
	: enumerator                         { $$ = $1; }
	| enumerator ',' enumerator_list     { $$ = ast_append($1, $3); }
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
    : declaration_specifiers declarator                { $$ =  }
	| declaration_specifiers abstract_declarator 
	| declaration_specifiers
	;

/* Obsolete-style declarator 
identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;
*/

// https://docs.microsoft.com/en-us/cpp/c-language/c-abstract-declarators?view=vs-2019
// An abstract declarator is a declarator without an identifier, consisting of one or more pointer, array, or function modifiers
abstract_declarator
	: pointer                               { $$ = $1; }
	| direct_abstract_declarator            { $$ = $1; }
	| pointer direct_abstract_declarator    { $$ = make_declaration($1, $2);  }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'                                { $$ = $2; }
	| direct_abstract_declarator '[' ']'                         { $$ = make_extent_direct_declarator($1, TP_ARRAY, NULL); }
	| direct_abstract_declarator '[' constant_expression ']'     { $$ = make_extent_direct_declarator($1, TP_ARRAY, $3); }
	| direct_abstract_declarator '(' parameter_list ')'      { $$ = make_extent_direct_declarator($1, TP_FUNC, $3); }
	| '(' parameter_list ')'                                 { $$ = make_extent_direct_declarator(NULL, TP_FUNC, $2); }
	;

initializer
    : assignment_expression         { $$ = $1; }
    | '{' initializer_list '}'      { $$ = make_initializer_list($2); }
	| '{' initializer_list ',' '}'  { $$ = make_initializer_list($2); }
    ;

initializer_list
    : initializer                         { $$ = $1; }
    | initializer ',' initializer_list    { $$ = ast_append($1, $3); }
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
/*
function_specifier
    : INLINE        { $$ = ATTR_INLINE; }
	;
*/
// statement part

statement
    : labeled_statement    { $$ = $1; }
    | compound_statement   { $$ = $1; }
    | expression_statement  { $$ = $1; }
    | selection_statement   { $$ = $1; }
    | iteration_statement  { $$ = $1; }
    | jump_statement       { $$ = $1; }
    ;

compound_statement
    : '{' '}'                         { $$ = make_empty(); }
    | '{' { ast_notify_enter_block(); } block_item_list '}'         { $$ = make_block($3); }
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
	: IDENTIFIER { notify_label($1); } ':' statement                { $$ = make_label($1, $4); }
	| CASE    { notify_label(NULL); } constant_expression ':' statement  { $$ = make_label_case($3, $5); }
	| DEFAULT { notify_label(NULL); } ':' statement                   { $$ = make_label_default($4); }
	;

expression_statement
    : ';'                                     { $$ = make_empty(); }
    | expression ';'                            { $$ = $1; }
    ;

selection_statement
	: IF '(' expression ')' statement                  { $$ = make_ifelse($3, $5, NULL); }
	| IF '(' expression ')' statement ELSE statement   { $$ = make_ifelse($3, $5, $7); }
	| SWITCH '(' expression ')' statement              { $$ = make_switch($3, $5); }
	;


iteration_statement
	: WHILE { notify_loop(LOOP_WHILE); } '(' expression ')' statement                                         { $$ = make_loop($4, NULL, $6, NULL); }
	| DO    { notify_loop(LOOP_DOWHILE); }   statement WHILE '(' expression ')' ';'                           { $$ = make_loop($6, NULL, $3, NULL); }
	| FOR   { notify_loop(LOOP_FOR); } '(' expression_statement expression_statement ')' statement            { $$ = make_loop($5, $4, $7, NULL); }
	| FOR   { notify_loop(LOOP_FOR); } '(' expression_statement expression_statement expression ')' statement { $$ = make_loop($5, $4, $8, $6); }
	| FOR   { notify_loop(LOOP_FOR); } '(' declaration expression_statement ')' statement                     { $$ = make_loop($5, $4, $7, NULL); }
	| FOR   { notify_loop(LOOP_FOR); } '(' declaration expression_statement expression ')' statement          { $$ = make_loop($5, $4, $8, $6); }
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
	: assignment_expression {$$ = $1;}
	| expression ',' assignment_expression { $$ = ast_append($1, $3);}
	;

assignment_expression
	: conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression { $$ = make_binary_expr($2, $1, $3); }
	;

assignment_operator
	: '='
	| MUL_ASSIGN { $$ = OP_ASSIGN; }
	| DIV_ASSIGN { $$ = OP_ASSIGN_DIV; }
	| MOD_ASSIGN { $$ = OP_ASSIGN_MOD; }
	| ADD_ASSIGN { $$ = OP_ASSIGN_ADD; }
	| SUB_ASSIGN { $$ = OP_ASSIGN_SUB; }
	| LEFT_ASSIGN { $$ = OP_ASSIGN_SHL; }
	| RIGHT_ASSIGN { $$ = OP_ASSIGN_SHR; }
	| AND_ASSIGN { $$ = OP_ASSIGN_AND; }
	| XOR_ASSIGN { $$ = OP_ASSIGN_XOR; }
	| OR_ASSIGN { $$ = OP_ASSIGN_OR; }
	;

constant_expression
	: conditional_expression
	;

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression { $$ = make_trinary_expr(OP_CONDITIONAL, $1, $3, $5 ); }
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = make_binary_expr(OP_OR, $1, $3); }
	;

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression {$$ = make_binary_expr(OP_AND, $1, $3); }
	;

exclusive_or_expression
	: and_expression { $$ = $1; }
	| exclusive_or_expression '^' and_expression { $$ = make_binary_expr(OP_BIT_XOR, $1, $3); }
	;

inclusive_or_expression
	: exclusive_or_expression {$$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression { $$ = make_binary_expr(OP_BIT_OR, $1, $3); }
	;

and_expression
	: equality_expression { $$ =$1; }
	| and_expression '&' equality_expression {$$ = make_binary_expr(OP_BIT_AND, $1, $3); }
    ;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = make_binary_expr(OP_EQUAL, $1, $3); }
	| equality_expression NE_OP relational_expression { $$ = make_binary_expr(OP_NOT_EQUAL, $1, $3); }
	;

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression '<' shift_expression { $$ = make_binary_expr(OP_LESS, $1, $3); }
	| relational_expression '>' shift_expression { $$ = make_binary_expr(OP_GREATER, $1, $3); }
	| relational_expression LE_OP shift_expression { $$ = make_binary_expr(OP_LESS_OR_EQUAL, $1, $3); }
	| relational_expression GE_OP shift_expression { $$ = make_binary_expr(OP_GREATER_OR_EQUAL, $1, $3); }
	;

shift_expression
	: additive_expression { $$ = $1; }
	| shift_expression LEFT_OP additive_expression { $$ = make_binary_expr(OP_SHIFT_LEFT, $1, $3); }
	| shift_expression RIGHT_OP additive_expression { $$ = make_binary_expr(OP_SHIFT_RIGHT, $1, $3); }
	;

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = make_binary_expr(OP_ADD, $1, $3); }
	| additive_expression '-' multiplicative_expression { $$ = make_binary_expr(OP_SUB, $1, $3); }
	;

multiplicative_expression
	: cast_expression { $$ = $1; }
	| multiplicative_expression '*' cast_expression { $$ = make_binary_expr(OP_MUL, $1, $3); }
	| multiplicative_expression '/' cast_expression { $$ = make_binary_expr(OP_DIV, $1, $3); }
	| multiplicative_expression '%' cast_expression { $$ = make_binary_expr(OP_MOD, $1, $3); }
	;

cast_expression
	: unary_expression { $$ = $1; }
	| '(' type_name ')' cast_expression { $$ = make_binary_expr(OP_CAST, $2, $4);}
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| INC_OP unary_expression { $$ = make_unary_expr(OP_INC, $2); }
	| DEC_OP unary_expression { $$ = make_unary_expr(OP_DEC, $2); }
	| unary_operator cast_expression { $$ = make_unary_expr($1, $2); }
	| SIZEOF unary_expression { $$ = make_unary_expr(OP_SIZEOF, $2); }
	| SIZEOF '(' type_name ')' { $$ = make_unary_expr(OP_SIZEOF, $3); }
	;

unary_operator
	: '&' { return OP_ADDRESS; }
	| '*' { return OP_POINTER; }
	| '+' { return OP_POSITIVE; }
	| '-' { return OP_NEGATICE; }
	| '~' { return OP_COMPLEMENT; }
	| '!' { reutrn OP_NOT; }
	;

postfix_expression
	: primary_expression { $$ = $1; }
	| postfix_expression '[' expression ']'  { $$ = make_binary_expr(OP_ARRAY_ACCESS, $1, $3); }
	| postfix_expression '(' ')'             
	| postfix_expression '.' IDENTIFIER { $$ = make_binary_expr(OP_STACK_ACCESS, $1, $3); }
	| postfix_expression PTR_OP IDENTIFIER { $$ = make_binary_expr(OP_PTR_ACCESS, $1, $3); }
	| postfix_expression INC_OP { $$ = make_unary_expr(OP_INC, $1); }
	| postfix_expression DEC_OP { $$ = make_unary_expr(OP_DEC, $1); }
	;

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'  { $$ = $2; }
	;

