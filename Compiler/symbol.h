#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"
#include "hashtable.h"
#include <stdio.h>
#include <llvm-c/Core.h>

STRUCT_TYPE(Symbol)

struct CompileData
{
	LLVMValueRef val;
};



/*
struct TypeAlias
{
	char* alias;
	int is_alias;

	struct TypeInfo* info;
	struct TypeAlias* prev;
	struct TypeAlias* next;
};
STRUCT_TYPE(TypeAlias)
*/

// 注意 类型名称 'struct A;'
//                     ^
//                     +------- 只能由一个空格
// type_name
//         : struct typename
//         : union  typename
//         : typename
struct TypeInfo
{
	char* type_name;     // 类型的名称 (必须是 struct TypeInfo 第一个字段)
	void* value;

	int is_alias;   
	char* field_name;    // struct/union 字段名称
	enum Types type;     // 基础类型

	int alignment;       // 最小对齐要求, bytes
	int aligned_size;    // 对齐后所占的空间, bytes
	int offset;          // 如果是结构体, 距离结构体首部的距离
	int bitfield_offset; // bit field 距离上一个非bitfield元素的距离
	int bitfield;        // bit field 占多少个字节
	int incomplete;

	enum SymbolAttributes qualifiers; // const 等 qualifier

	union
	{
		struct Basic {
			struct TypeInfo* child;
		};

		struct StructOrUnion
		{
			struct TypeInfo* child;
		} struc ;

		struct Pointer
		{
			struct TypeInfo* pointing;
			int indirection;
		} ptr ;

		struct Array
		{
			struct TypeInfo* array_type;
			uint64_t array_count;
			int has_value;
		} arr;
		
		struct Function
		{
			struct TypeInfo* return_type;
			struct TypeInfo* params;
		} fn ;
		/*
		struct EnumItem
		{
			void* placeholder;
			uint64_t val;
			int has_value;
		} enu;
		*/
		struct StructField
		{
			void* placeholder;
			enum SymbolAttributes attrib;
		} struc_field;
	};
	
	struct TypeInfo* prev;
	struct TypeInfo* next;

	struct TypeAlias* alias;
	struct TypeInfo* alias_origin;
};
STRUCT_TYPE(TypeInfo)

struct VariableInfo
{
	char* name;
	void* value;

	enum SymbolAttributes attributes;
	Symbol* type;     // Typeinfo of this symbol
	int is_constant;  // is constant value, c++ constexpr

	struct VariableInfo* prev;
	struct VariableInfo* next;
};
STRUCT_TYPE(VariableInfo)

struct FunctionInfo
{
	char* name;
	LLVMValueRef value;

	// TypeInfo* return_type;
	LLVMTypeRef ret_type;
	LLVMTypeRef* params;
	struct AST* body;
};
STRUCT_TYPE(FunctionInfo)

struct LabelInfo
{
	char* name;
	void* value;

	uint64_t label_id;
	int resolved; // 当 goto 定义在 label: 之前的时候, 会产生一个 unresolved label
};
STRUCT_TYPE(LabelInfo)


struct SymbolBase
{
	char* key;
};

enum SymbolTypes
{
	Symbol_TypeInfo,
	Symbol_VariableInfo,
	Symbol_FunctionInfo,
	Symbol_LabelInfo
};
struct Symbol
{
	union 
	{
		struct
		{
			char* name;    // name of the symbol
			void* value;   // value of the symbol, used for evaluation
		};                 // all symbol must have these 2 fields as first 2 fields
		TypeInfo type;     // type info
		VariableInfo var;  // variables, including constants like enums
		FunctionInfo func; // functions
		LabelInfo label;   // labels
	};

	Symbol* prev;
	enum SymbolTypes usage; // used for determine which kind of symbo this is.
};



struct SymbolStackInfo
{
	Symbol* first;                      // first symbol in stack
	Symbol* last;                       // last symbol in stack

	struct SymbolStackInfo* prev;       // previous stack
	struct SymbolStackInfo* next;       // next stack

	struct SymbolStackInfo* real_prev;  // some stack may not use last symbol, espcially for inline funtions
	int nest;                           // level of nesting
};

// TODO: Use hash table to optimize
struct SymbolTable
{
	struct SymbolStackInfo* stack_top;
	struct SymbolStackInfo* bottom;
	
	//struct SymbolStackInfo* global;
};

STRUCT_TYPE(SymbolTable)


void symbol_init_context(struct Context* context);

// 符号表相关函数
// ================================
SymbolTable* symtbl_new();
// Symbol.name 记录了 name 的信息
void symtbl_push(SymbolTable* tbl, Symbol* c);
Symbol* symtbl_find(SymbolTable* tbl, const char* name);
Symbol* symtbl_find_in_current_scope(SymbolTable* tbl, const char* name);
// 如果进入函数的话, 那么函数只能访问全局变量, 所以要 keep_global_only = true
void symtbl_enter_scope(SymbolTable* tbl, int keep_global_only);
void symtbl_leave_scope(SymbolTable* tbl, int free_all_symols);

// 符号创建函数
// ================================
TypeInfo* type_fetch_buildtin(enum Types type);
Symbol* symbol_create_label(char* name, uint64_t label, int resolved);
Symbol* symbol_create_constant(Symbol* enum_sym, char* name, void* val);
Symbol* symbol_create_enum(char* name);
Symbol* symbol_create_enum_item(Symbol* type, Symbol* prev, char* name, void* val);
Symbol* symbol_from_type_info(TypeInfo* info);
Symbol* symbol_create_struct_or_union(TypeInfo* info, TypeInfo* child);
Symbol* symbol_create_struct_or_union_incomplete(char* name, enum Types struct_or_union);
Symbol* symbol_create_func(char* name, LLVMValueRef f, LLVMTypeRef ret_type, LLVMTypeRef* params, struct AST* body);
Symbol* symbol_create_variable(char* name, enum SymbolAttributes attributes, Symbol* type, void* value, int is_constant);

// 类型管理 & 创建
// ================================
TypeInfo* type_create_array(uint64_t n, enum SymbolAttributes qualifers, TypeInfo* array_element_type);
// TypeInfo* type_create_struct_or_union(enum Types type, char* name);
TypeInfo* type_create_ptr(enum SymbolAttributes qualifers, struct TypeInfo* pointing);
TypeInfo* type_create_func(struct TypeInfo* ret, char* name, struct TypeInfo* params);
TypeInfo* create_struct_field(TypeInfo* type_info, enum SymbolAttributes attributes, char* field_name);
// TypeInfo* create_struct_bitfield(TypeInfo* type_info, enum SymbolAttributes attributes, char* field_name, int bits);

TypeInfo* type_create_param_ellipse();

void variable_append(Symbol* last, Symbol* new_last);

// int type_wrap(TypeInfo* parent, TypeInfo* child);
int type_append(TypeInfo* tail, TypeInfo* new_tail);
int type_is_builtin(enum Types type);

TypeInfo* type_get_child(TypeInfo* parent);
TypeInfo* type_get_error_type();


// 类型工具
// ================================

// 把一个任意的 numeric 类型 cast 成别的 numeric 类型
union ConstantValue constant_cast(enum Types from, enum Types to, union ConstantValue src);

inline int type_is_number(int type)
{
	int x = (type & (0x0010 - 1));
	return (x >= TP_INT8) && (x <= TP_FLOAT128);
}

inline int type_native_alignment(int type)
{
	int x = (type & (0x0010 - 1));
	return (x >= TP_INT8) && (x <= TP_FLOAT128);
}

int type_is_interger(enum Types type);


void value_constant_print(FILE* f, enum Types type, union ConstantValue* pval);

#endif