#ifndef CC_AST_H
#define CC_AST_H
#include <stdint.h>
#include <stdlib.h>
#include "config.h"
#include "symbol.h"

#define AST_NODE_LIST(V) \
	V(BlockExpr) \
	V(ReturnStatement) \
	V(ListExpr) \
	V(FunctionCallExpr)\
    V(NumberExpr)   \
	V(SymbolExpr)   \
	V(FunctionExpr) \
	V(OperatorExpr) \

#define AST_AUX_NODE_LIST(V)\
	V(EmptyExpr) \
	V(TypenameExpr)\
	V(LoopStmt)\
	V(IfStmt) \
	V(SwitchCaseStmt)\
	V(DeclaratorExpr)


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
	enum Types type;
	Symbol* sym_type;
	struct AST* prev, * next;
};
typedef struct AST AST;


typedef struct FunctionDefinition
{

	AST* params;
	int n_params;
	const char* name;
	AST* body;
	const char* return_type;
} FunctionDefinition;

//
// { first_child; second; ... }
// 
struct BlockExpr
{
	AST super;

	AST* first_child;
};

struct ReturnStatement
{
	AST super;

	AST* return_val;
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
};


// { 1, 2, 3, 4 ..}
struct InitilizerListExpr
{
	AST super;
	AST* vals;
	void* evaluated_value;
};

//   struct/union { fields };
struct AggregateDeclareStmt
{
	AST super;
	Symbol* ref;
	AST* fields;
};

// enum { enums }
struct EnumDeclareStmt
{
	AST super;
	Symbol* ref;
	AST* enums;
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

	AST* params;
	int n_params;
	const char* function_name;
};




// goto label;
// continue;
// break;
struct JumpStmt
{
	AST super;
	enum JumpType type;
	AST* target;
	const char* label;
};


// next_loop:       <-- label
//      io += 10;   <-- target
struct LabelStmt
{
	AST super;
	const char* label;
	AST* target;
};


// TODO: Rename as constant
struct NumberExpr
{
	AST super;

	enum Types number_type;
	union
	{
		int32_t i32;
		uint32_t ui32;
		int64_t i64;
		uint64_t ui64;
		float f32;
		double f64;
		char* str;
	};
};


// x
/*
struct SymbolExpr
{
	AST super;
	Symbol* type;
	Symbol* name;
};
*/

struct SymbolExpr
{
	AST super;
	Symbol* ref;
};



struct FunctionExpr
{
	AST super;
	FunctionDefinition* ref;
};


// Auxiliary AST Nodes
// ================================
// 辅助 AST 节点, 便于 yacc 调用时生成,
// 不会交给代码生成器, 会被翻译成其他的节点

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

	AST* enter; // 进入循环前执行的代码
	AST* condition; // 循环条件
	AST* step; // 每一步要做的操作
	AST* body;
};

struct SwitchCaseStmt
{
	AST super;
	AST* switch_value;
	AST* cases;
};

struct DeclaratorExpr
{
	AST super;
	int indirection;
	char* name;
};

struct TypenameExpr
{
	AST super;
	enum SymbolAttributes attr;
	enum Types type;
	char* name;
};

struct DeclarationExpr
{
	AST super;
	Symbol* sym_type;
	AST* declarators;
};



#define FORWORD_DECL(x) STRUCT_TYPE(x)
AST_NODE_LIST(FORWORD_DECL)
AST_AUX_NODE_LIST(FORWORD_DECL)
#undef FORWORD_DECL


void ast_init(AST* ast, ASTType type);

// argument-expression-list
AST* ast_append(AST* leader, AST* follower);

// Expressions
// =======================================

// primary-expression:
//    identifier
// 变量声明
AST* make_identifier(const char* c);

// primary-expression:
//    constants
//    string-literal
// 常量定义, 注意函数会自动释放内存
AST* make_number_int(char* c, enum Types type);
AST* make_number_float(char* c, int bits);
AST* make_string(char* c);


// conditional-expression
// assignment-expression
// postfix-expression
// 
// n-nary operator expr
// 赋值, 条件, 比较, 比特运算, 取址, 访问元素等
AST* make_unary_expr(enum Operators unary_op, AST* rhs);
AST* make_binary_expr(enum Operators binary_op, AST* lhs, AST* rhs);
AST* make_trinary_expr(enum Operators triary_op, AST* cond, AST* lhs, AST* rhs);


// Statements
// ======================================
AST* make_block(AST* statements);

AST* make_label(char* name, AST* statement);
AST* make_label_case(AST* constant, AST* statements);
AST* make_label_default(AST* statements);

// goto name
// continue
// break
// return ret
AST* make_jump(enum JumpType type, char* name, AST* ret);

AST* make_declare_array(AST* direct, AST* constant);
AST* make_declare_func(AST* direct, AST* constant);
AST* make_declare_pointer(enum SymbolAttributes type_qulifiers);
AST* make_declare_declarator(AST* pointer, AST* declarator);

// while(condition) loop_body
// for(before_loop; condition; loop_step) loop_body;
// do loop_body while(condition)
AST* make_loop(AST* condition, AST* before_loop, AST* loop_body, AST* loop_step, enum LoopType loop_type);

// if(condition) then else otherwise
AST* make_ifelse(AST* condition, AST* then, AST* otherwise);

AST* make_switch(AST* condition, AST* body);



// Declarations
// =================================

// attr 是 register/auto/extern/static
// type 是 TP_VOID ~ TP_FLOAT128 等数字类型的时候, name = NULL
// type 是 TP_STRUCT/TP_UNION/TP_ENUM 等, 那么 struct/union/enum的名字作为 name
// type 是 TP_INCOMPLETE, 说明是用户自定义的类型 (通过 typedef)
// name 会被 AST 接管(由AST负责释放)
AST* make_type(enum SymbolAttributes attr, enum Types type, const char* name);

// typedef target new_name
AST* make_typedef(AST* target, const char* new_name);

// attr 是 __cdecl, __stdcall, inline
AST* make_type_add_attr(AST*target, enum SymbolAttributes attr);

// qualifier 是 const/volatile
// pointer:
//    '*' type-qualifier-list?
//    '*' type-qualifier-list? pointer
AST* make_ptr(enum Types qualifier, AST* pointing);


// declarator:
//    pointer? direct_declarator
// direct-declarator:
//    identifier
//    ( declarator )
//    direct-declarator [ constant-expressionopt ]
//    direct-declarator ( parameter-type-list )  
AST* make_declarator(AST* pointer, AST* direct_declarator);

// enum_define:
//     enum identifier { enum_list }
AST* make_enum_define(char* identifier, AST* enum_list);

// struct_or_union_define:
//     struct/union identifier { field_list }
AST* make_struct_or_union_define(enum Types type, char* identifier, AST* field_list);


AST* make_symbol(const char* type, const char* name);



AST* make_function_call(const char* function_name, AST* params);

AST* make_function(const char* return_type, const char* name, AST* arg_list);

AST* make_function_body(AST* function_ast_node, AST* body);

AST* make_return(AST* exp);

AST* make_empty();

FunctionDefinition* sym_get_function(const char* return_type, const char* name, AST* args);

#define TYPECHECK(x) inline int is_##x(AST* ast) { return ast->type == AST_##x; }

// AST_NODE_LIST(TYPECHECK);

#undef TYPECHECK

#endif


