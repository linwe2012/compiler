%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token BOOL COMPLEX IMAGINARY
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start translation_unit
%%

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
    : CONST
    | VOLATILE
    | RESTRICT
    ;
function_specifier
    : INLINE;

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
    