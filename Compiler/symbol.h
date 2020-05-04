#ifndef _SYMBOL_H_
#define _SYMBOL_H_


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

int type_number_size(int type);


struct Symbol
{
	int type;
	int attributes;
};

struct TypeDeclaration
{
	
};



struct AST
{
	const char* filename;
	int line;
	

};

#endif