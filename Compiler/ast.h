#ifndef CC_AST_H
#define CC_AST_H
#include <stdint.h>
#include <stdlib.h>
#include "config.h"
#define AST_PARAMS 

enum Types
{
	TP_VOID = 0u,
	TP_INT8, // signed char
	TP_INT16, // short
	TP_INT32, // int
	TP_INT64, // long long
	TP_INT128,

	TP_FLOAT32, // float
	TP_FLOAT64, // double
	TP_FLOAT128,

	TP_STRUCT = 0x0010u,
	TP_UNION  = 0x0020u,
	TP_ENUM   = 0x0040u,
	TP_PTR    = 0x0080u, // pointer
	TP_FUNC   = 0x0100u, // function

	TP_UNSIGNED = 0x001000u,
	TP_SIGNED   = 0x002000u,
	TP_ARRAY    = 0x004000u,
	TP_BITFIELD = 0x008000u,
	TP_CONST    = 0x010000u,
	TP_VOLATILE = 0x020000u,

	TP_CLEAR_SIGNFLAGS = ~(TP_UNSIGNED | TP_SIGNED),
	TP_CLEAR_ATTRIBUTEFLAGS = (TP_UNSIGNED - 1)
};

// typename, typename in c,  bits
#define INTERNAL_TYPE_LIST(V)\
V(INT8, "char", 8)\
V(INT16, "short", 16)\
V(INT32, "int", 32) \
V(INT64, "long long", 64)\
V(INT128, "__int128", 128) \
V(FLOAT32, "float", 32)\
V(FLOAT64, "double", 64)\
V(FLOAT128, "__float128", 128)


inline int type_numeric_bytes(enum Types t)
{
#define TYPE_BYTES(t, cstr, bits) case TP_##t: return bits / 8;
	switch (t)
	{
		INTERNAL_TYPE_LIST(TYPE_BYTES)
	default:
		break;
	}
#undef TYPE_BYTES
	return -1;
}

inline int type_is_float_point(enum Types t)
{
	return t <= TP_FLOAT128 && t >= TP_FLOAT32;
}


#define AST_NODE_LIST(V) \
	V(EmptyExpr) \
	V(BlockExpr) \
	V(ReturnStatement) \
	V(ListExpr) \
	V(FunctionCallExpr)\
    V(NumberExpr) \
	V(SymbolExpr) \
	V(FunctionExpr)


typedef enum ASTType {
#define ENUM_AST(x) AST_##x,
	AST_NODE_LIST(ENUM_AST)
#undef ENUM_AST
} ASTType;


struct AST {
	ASTType type;
	struct AST* prev, * next;
};
typedef struct AST AST;


struct Symbol
{
	const char* name;
};
typedef struct Symbol Symbol;

typedef struct FunctionDefinition
{
	AST* params;
	int n_params;
	const char* name;
	AST* body;
	const char* return_type;
} FunctionDefinition;


// 方便 yacc
struct EmptyExpr
{

	AST super;

};

//
// { BlockNode }
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

// a, b, c, d
// ^
// +-------- first child
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

// if(condition) then; else otherwise;
struct IfStatement
{
	AST super;

	AST* condition;
	AST* then;
	AST* otherwise;
};

// while (condition) body;
// for(enter; condition; step) body;
struct LoopStatement
{
	AST super;

	AST* enter; // 进入循环前执行的代码
	AST* condition; // 循环条件
	AST* step; // 每一步
	AST* body;
};



struct GotoStatement
{
	AST super;
	const char* label;
};

// next_loop:       <-- label
//      io += 10;   <-- target
struct LabelStatement
{
	AST super;
	const char* label;
	AST* target;
};

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
	};
};

struct UnionOrStructExpr
{
	AST supr;
	Symbol* ref;
};

// int x;
struct SymbolExpr
{
	AST super;
	Symbol* type;
	Symbol* name;
};

struct FunctionExpr
{
	AST super;
	FunctionDefinition* ref;
};


#define FORWORD_DECL(x) STRUCT_TYPE(x)
AST_NODE_LIST(FORWORD_DECL)
#undef FORWORD_DECL

void ast_init(AST* ast, ASTType type);

AST* ast_append(AST* leader, AST* follower);

AST* make_number_int32(const char* c);

AST* make_number_float(const char* c, int bits);

AST* make_identifier(const char* c);

Symbol* sym_get_type(const char* n);

Symbol* sym_get_identifier(const char* n);

FunctionDefinition* sym_get_function(const char* return_type, const char* name, AST* args);

AST* make_symbol(const char* type, const char* name);

AST* make_block(AST* first_child);

AST* make_function_call(const char* function_name, AST* params);

AST* make_function(const char* return_type, const char* name, AST* arg_list);

AST* make_function_body(AST* function_ast_node, AST* body);

AST* make_return(AST* exp);

AST* make_empty();

#define TYPECHECK(x) inline int is_##x(AST* ast) { return ast->type == AST_##x; }

// AST_NODE_LIST(TYPECHECK);

#undef TYPECHECK

#endif


