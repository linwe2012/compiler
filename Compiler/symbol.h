#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "config.h"
#include <stdlib.h>
#include <string.h>
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

	int alignment;
	int aligned_size;
	int offset; // 如果是结构体, 距离结构体首部的距离

	union
	{
		struct StructOrUnion
		{
			struct TypeInfo* child;
		};

		struct Pointer
		{
			TypeInfo* pointing;
			int indirection;
		};

		struct Array
		{
			struct TypeInfo* array_type;
			uint64_t array_count;
		};
		
		struct Function
		{
			TypeInfo* return_type;
			TypeInfo* params;
		};

	};
	
	
	int indirection; // 指针的层级, 比如 int*** 的 inderection = 3
	struct TypeInfo* child;

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

TypeInfo* type_append(TypeInfo* left, TypeInfo* right)
{

}

struct SymbolBase
{
	char* key;
};

enum SymbolTypes
{
	Symbol_TypeInfo,
	Symbol_VariableInfo,
	Symbol_FunctionInfo
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
	};

	Symbol* prev;
	Symbol* next;
	enum SymbolTypes usage;
};
STRUCT_TYPE(Symbol)

struct SymbolTableEntry
{
	HashTable* table;
	struct SymbolTableEntry* prev;
	struct SymbolTableEntry* next;
	int hide_bottom;
	Symbol* first;
	Symbol* last;
};

struct SymbolTable
{
	struct SymbolTableEntry* stack_bottom;
	struct SymbolTableEntry* stack_top;
	struct SymbolTableEntry* global;
};
STRUCT_TYPE(SymbolTable)

extern Symbol SymbolType_UINT64;



#endif