#ifndef CC_AST_H
#define CC_AST_H
#include <stdint.h>
#include <stdlib.h>
#include "config.h"
#include "symbol.h"
#include "types.h"

#define AST_NODE_LIST_STMT(V) \
	V(LabelStmt) \
	V(JumpStmt) \
	V(DeclareStmt)\
	V(EnumDeclareStmt)\
	V(AggregateDeclareStmt)\
	V(FunctionDefinitionStmt)

#define AST_NODE_LIST_EXPR(V) \
	V(BlockExpr) \
	V(ListExpr) \
	V(FunctionCallExpr)\
	V(IdentifierExpr) \
	V(NumberExpr)   \
	V(OperatorExpr) \
	V(InitilizerListExpr)

#define AST_NODE_LIST(V) \
	AST_NODE_LIST_STMT(V) \
	AST_NODE_LIST_EXPR(V)

#define AST_AUX_NODE_LIST(V)\
	V(EmptyExpr) \
	V(LoopStmt)\
	V(IfStmt) \
	V(SwitchCaseStmt)\
	V(DeclaratorExpr)\
	V(TypeSpecifier)


typedef enum ASTType {
#define ENUM_AST(x) AST_##x,
	AST_NODE_LIST(ENUM_AST)
	AST_AUX_NODE_LIST(ENUM_AST)
#undef ENUM_AST
} ASTType;


struct ASTVPTR
{
	int (*is_constexpr)();

};

struct AST {
	ASTType type;
	struct AST* prev, * next;
	struct SematicData* sematic;
};
typedef struct AST AST;

// 
// 用大括号包围的语句会进入一个新的 scope
// { first_child; second; ... }
// 
struct BlockExpr
{
	AST super;

	AST* first_child;
};


struct FunctionDefinitionStmt
{
	AST super;

	AST* declarator;
	AST* body;
	AST* specifier;
};

//
// OperationExpr
//      : unary_expr(op, prefix_or_postfix, rhs)   <-- ++rhs
//      | binary_expr(op, lhs, rhs)        <-- lhs + rhs
//      | trinary_expr(op, cond, lhs, rhs) <--  cond ? lhs : rhs
//      | cast_expr(type, expr)            <-- (type) expr
//
struct OperatorExpr
{
	AST super;

	enum Operators op;
	AST* lhs;
	AST* rhs;
	AST* cond;
	enum Types number_type;
	
	uint64_t _barrier; // 这个主要为了帮助我们获取指向 Union 的指针, 因为内存对齐的要求, 它必须是 int64
};


//
// DeclareStmt
//         : type identifier
//         : type assignment*    <-- int a = 10, b=20;
struct DeclareStmt
{
	AST super;
	AST* type;
	AST* identifiers;
	enum SymbolAttributes attributes;
};

//   struct/union { fields };
struct AggregateDeclareStmt
{
	enum Types type; // TP_STRUCT/TP_UNION
	AST super;
	AST* fields;
	char* name;

	Symbol* ref;
};

// enum { enums }
struct EnumDeclareStmt
{
	AST super;
	AST* enums;
	char* name;
	Symbol* ref;
};


// a, b, c, d
// ^
// +-------- first child
// List expr 的值是最后一个 child
struct ListExpr
{
	AST super;
	AST* first_child;
};


// call(a, b, ...)
struct FunctionCallExpr
{
	AST super;

	AST* function;
	AST* params;
	int n_params;
	const char* function_name; // TODO: 这个要被删掉
};


// goto label;
// continue;
// break;
struct JumpStmt
{
	AST super;
	enum JumpType type;
	AST* target;
	Symbol* ref;
};


// next_loop:       <-- label
//      io += 10;   <-- target
struct LabelStmt
{
	AST super;
	AST* target;
	AST* condition; // < swtich case 中的 condition
	enum LabelType type;

	Symbol* ref;
};


// TODO: Rename as constant
struct NumberExpr
{
	AST super;

	enum Types number_type;
	union
	{
		int8_t i8;
		uint8_t ui8;
		int16_t i16;
		uint16_t ui16;
		int32_t i32;
		uint32_t ui32;
		int64_t i64;
		uint64_t ui64;
		float f32;
		double f64;
		char* str;
	};
};

struct IdentifierExpr
{
	AST super;
	char* name;
	AST* val;
};



// 方便 yacc
// 如果在解析出错的时候也返回 empry
struct EmptyExpr
{
	AST super;
	const char* error;
};

// if(condition) then; else otherwise;
struct IfStmt
{
	AST super;

	AST* condition;
	AST* then;
	AST* otherwise;
};


// while (condition) body;
// for(enter; condition; step) body;
struct LoopStmt
{
	AST super;
	enum LoopType loop_type;
	AST* enter; // 进入循环前执行的代码
	AST* condition; // 循环条件
	AST* step; // 每一步要做的操作
	AST* body;

	uint64_t cond_label;
	uint64_t exit_label;
	uint64_t step_label;
};

struct SwitchCaseStmt
{
	AST super;
	AST* switch_value;
	AST* cases;
	uint64_t exit_label;
};

// type name = init_value;
// init_value 可能是 算数表达式
//   也可能是初始化语句
struct DeclaratorExpr
{
	AST super;
	char* name;
	//TypeInfo* last;
	//TypeInfo* first;

	struct TypeSpecifier* type_spec;
	struct TypeSpecifier* type_spec_last;
	AST* bitfield;

	AST* init_value;
	enum SymbolAttributes attributes;
};


struct InitilizerListExpr
{
	AST super;
	AST* list;
};

// Auxiliary AST Nodes
// ================================
// 辅助 AST 节点, 便于 yacc 调用时生成,

enum TypeSpecifierFlags
{
	TypeSpecifier_None,
	TypeSpecifier_Unsigned = 1,
	TypeSpecifier_Signed = 2,
	TypeSpecifier_Long = 4,
	TypeSpecifier_LongLong = 8,
	TypeSpecifier_Exclusive = 16
};

struct TypeSpecifier
{
	AST super;
	char* name;
	char* field_name;
	enum Types type;
	int paren;
	
	struct TypeSpecifier* child;
	struct TypeSpecifier* params;

	enum SymbolAttributes attributes;

	enum TypeSpecifierFlags flags;
	AST* array_element_count;
	AST* struct_or_union;
};


#define FORWORD_DECL(x) STRUCT_TYPE(x)
AST_NODE_LIST(FORWORD_DECL)
AST_AUX_NODE_LIST(FORWORD_DECL)
#undef FORWORD_DECL

void ast_init_context(struct Context* _ctx);


void ast_init(AST* ast, ASTType type);

// argument-expression-list
AST* ast_append(AST* leader, AST* follower);

AST* make_empty();
AST* make_error(const char* message);

// Expressions
// =======================================

// primary-expression:
//    identifier
// 变量声明
AST* make_identifier(const char* c);
AST* make_identifier_with_constant_val(const char* c, AST* constant_val);

// primary-expression:
//    constants
//    string-literal
// 常量定义, 注意函数会自动释放内存
AST* make_number_int(char* c, enum Types type);
AST* make_number_float(const char* c, int bits);
AST* make_string(char* c);
AST* make_char(char* c);


// conditional-expression
// assignment-expression
// postfix-expression
// 
// n-nary operator expr
// 赋值, 条件, 比较, 比特运算, 取址, 访问元素等
AST* make_unary_expr(enum Operators unary_op, AST* rhs);
AST* make_binary_expr(enum Operators binary_op, AST* lhs, AST* rhs);
AST* make_trinary_expr(enum Operators triary_op, AST* cond, AST* lhs, AST* rhs);

AST* make_list_expr(AST* child);

// Statements
// ======================================
AST* make_block(AST* statements);
void ast_notify_enter_block();

AST* make_label(char* name, AST* statement);
AST* make_label_case(AST* constant, AST* statements);
AST* make_label_default(AST* statements);
void notify_label(char* name);

// goto name
// continue
// break
// return ret
AST* make_jump(enum JumpType type, char* name, AST* ret);

// while(condition) loop_body
// for(before_loop; condition; loop_step) loop_body;
// do loop_body while(condition)
AST* make_loop(AST* condition, AST* before_loop, AST* loop_body, AST* loop_step, enum LoopType loop_type);
void notify_loop(enum LoopType type);

// if(condition) then else otherwise
AST* make_ifelse(AST* condition, AST* then, AST* otherwise);

// switch ( condition ) { body }
AST* make_switch(AST* condition, AST* body);

AST* make_parameter_declaration(AST* declaration_specifiers, AST* declarator);

AST* make_define_function(AST* declaration_specifiers, AST* declarator, AST* compound_statement);

// attr 是 register/auto/extern/static
// type 是 TP_VOID ~ TP_FLOAT128 等数字类型的时候, name = NULL
// type 是 TP_STRUCT/TP_UNION/TP_ENUM 等, 那么 struct/union/enum的名字作为 name
// type 是 TP_INCOMPLETE, 说明是用户自定义的类型 (通过 typedef)
// name 会被 AST 接管(由AST负责释放)
// AST* make_type(enum SymbolAttributes attr, enum Types type, const char* name);




// Declarations
// =================================

// qualifier 是 const/volatile/restrict
// pointer:
//    '*' type-qualifier-list?
//    '*' type-qualifier-list? pointer
AST* make_ptr(int type_qualifier_list, AST* pointing);
AST* make_declaration(AST* declaration_specifiers, enum SymbolAttributes attribute_specifier, AST* init_declarator_list);

// direct_declarator
//     : IDENTIFIER
AST* make_init_direct_declarator(const char * name);

// direct_declarator:
//     : direct_declarator (wrapped)
//     | direct_declarator [wrapped]
AST* make_extent_direct_declarator(AST* direct, enum Types type, AST* wrapped);


// declarator:
//    pointer? direct_declarator
AST* make_declarator(AST* pointer, AST* direct_declarator);

// init_declarator:
//       declarator = initializer
AST* make_declarator_with_init(AST* declarator, AST* init);
AST* make_declarator_bit_field(AST* declarator, AST* bitfield);


AST* make_type_specifier(enum Types type);
AST* make_type_specifier_extend(AST* me, AST*other, enum SymbolAttributes storage);
AST* make_type_specifier_from_id(char* id);
AST* ast_merge_specifier_qualifier(AST* me, AST* other, enum Types qualifier);
int ast_merge_type_qualifier(int a, int b);

// attr 是 __cdecl, __stdcall, inline

// enum_define:
//     enum identifier { enum_list }
AST* make_enum_define(char* identifier, AST* enum_list);

// struct_or_union_define:
//     struct/union identifier { field_list }
AST* make_struct_field_declaration(AST* specifier_qualifier, AST* struct_declarator);
AST* make_struct_or_union_define(enum Types type, char* identifier, AST* field_list);
AST* make_mark_declarator_paren(AST* target);



AST* make_initializer_list(AST* list);


AST* make_function_call(AST* postfix_expression, AST* params);
AST* make_type_declarator(AST* specifier_qualifier, AST* declarator);
AST* make_paramter_ellipse();


void do_eval(AST* ast, struct Context* _ctx, char* module_name, const char* output_file);

/*


AST* make_function(const char* return_type, const char* name, AST* arg_list);

AST* make_function_body(AST* function_ast_node, AST* body);

AST* make_return(AST* exp);
*/



#define TYPECHECK(x) inline int is_##x(AST* ast) { return ast->type == AST_##x; }

// AST_NODE_LIST(TYPECHECK);

#undef TYPECHECK

// Helper functions for sematic analysis
// ========================================

void extend_declarator_with_specifier(DeclaratorExpr* decl, TypeSpecifier* spec);


#endif


