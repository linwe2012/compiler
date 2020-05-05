#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "config.h"

enum SymbolAttributes
{
	ATTR_EXTERN,
	ATTR_STATIC,
	ATTR_TYPEDEF,
	ATTR_INLINE
};

enum Types
{
	TP_VOID,
	TP_INT8, // signed char
	TP_INT16, // short
	TP_INT32, // int
	TP_INT64, // long long
	TP_INT128,

	TP_FLOAT32, // float
	TP_FLOAT64, // double
	TP_FLOAT128,

	TP_STRUCT = 0x0010,
	TP_UNION,
	TP_ENUM,
	TP_PTR, // pointer
	TP_FUNC, // function

	TP_UNSIGNED = 0x00100,
	TP_SIGNED   = 0x00200,
	TP_ARRAY    = 0x00400,
	TP_BITFIELD = 0x00800,
	TP_CONST    = 0x01000,
	TP_VOLATILE = 0x02000,
};

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

struct TypeInfo
{
	char* type_name;
	char* field_name;
	enum Types type;
	int aligned_size;
	int alignment;
	int offset; // 如果是结构体, 距离结构体首部的距离
	struct TypeInfo* child;

	struct TypeInfo* prev;
	struct TypeInfo* next;
};
STRUCT_TYPE(TypeInfo)

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


struct SymbolType
{

};

struct Symbol
{
	int type;
	int attributes;
};

struct TypeDeclaration
{
	
};


#endif