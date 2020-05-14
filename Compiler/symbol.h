#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"
#include "hashtable.h"

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

int type_number_size(int type);

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
	char* type_name;
	int is_alias;
	char* field_name;
	enum Types type;

	int alignment;  // bytes
	int aligned_size; // bytes
	int offset; // 如果是结构体, 距离结构体首部的距离
	int bitfield_offset; // bit field 距离上一个非bitfield元素的距离
	int bitfield;        // 

	enum SymbolAttributes qualifiers;

	union
	{
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
		} arr;
		
		struct Function
		{
			struct TypeInfo* return_type;
			struct TypeInfo* params;
		} fn ;

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
	enum SymbolAttributes attributes;
	TypeInfo* type;
};
STRUCT_TYPE(VariableInfo)

struct FunctionInfo
{
	char* name;
	TypeInfo* return_type;
	TypeInfo* params;
	struct AST* body;
};
STRUCT_TYPE(FunctionInfo)

struct LabelInfo
{
	char* name;
	uint64_t label_id;
	int resolved; // 当 goto 定义在 label: 之前的时候, 会产生一个 unresolved label
};
STRUCT_TYPE(LabelInfo)


TypeInfo* type_access(TypeInfo* info, const char* name)
{
	
}

#define max(x, y) ((x) > (y) ? (x) : (y))

int type_add_child(TypeInfo* info, TypeInfo* child)
{
	TypeInfo* move = child;
	int max_aligned_size = 0;
	int max_alignment = 0;
	while (move)
	{
		max_aligned_size = max(move->aligned_size, max_aligned_size);
		max_alignment = max(move->alignment, max_alignment);
	}

	if (info->type & TP_UNION)
	{
		move->offset = 0;
		info->aligned_size = max_aligned_size;
		info->alignment = max_alignment;
	}
	else if (info->type & TP_STRUCT)
	{
		int size = 0;
		move = child;
		while (move)
		{
			if (size != 0)
			{
				size = (size / move->alignment + 1) * move->alignment;
			}
			
			move->offset = size;
			size += move->alignment;
		}


		info->aligned_size = size;
		info->alignment = max_alignment;
	}
	else if (info->type & TP_ENUM)
	{

	}
	


}

TypeInfo* type_create_array(uint64_t n, enum SymbolAttributes qualifers);
TypeInfo* type_create_struct();
TypeInfo* type_create_ptr(enum SymbolAttributes qualifers);
TypeInfo* type_create_func(struct TypeInfo* params);
int type_wrap(TypeInfo* parent, TypeInfo* child);

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
			char* name;
		};
		TypeInfo type;
		VariableInfo var;
		FunctionInfo func;
		LabelInfo* label;
	};

	Symbol* prev;
	// Symbol* next;
	enum SymbolTypes usage;
};
STRUCT_TYPE(Symbol)


struct SymbolStackInfo
{
	Symbol* first;
	Symbol* last;

	struct SymbolStackInfo* prev;
	struct SymbolStackInfo* next;

	struct SymbolStackInfo* real_prev;
	int nest;
};

// TODO: Use hash table to optimize
struct SymbolTable
{
	struct SymbolStackInfo* stack_top;
	struct SymbolStackInfo* bottom;
	
	//struct SymbolStackInfo* global;
};

STRUCT_TYPE(SymbolTable)

// void symtbl_remove_by_name(const char* name);

// Symbol.name 记录了 name 的信息
void symtbl_push(SymbolTable* tbl, Symbol* c);
Symbol* symtbl_find(SymbolTable* tbl, const char* name);
// 如果进入函数的话, 那么函数只能访问全局变量, 所以要 keep_global_only = true
void symtbl_enter_scope(SymbolTable* tbl, int keep_global_only);
void symtbl_leave_scope(SymbolTable* tbl, int free_all_symols);


extern Symbol SymbolType_UINT64;


extern TypeInfo* type_uint64;

void symbol_init_context(struct Context* context);
TypeInfo* type_fetch_buildtin(enum Types type);
Symbol* symbol_create_label(char* name, uint64_t label, int resolved);

#endif