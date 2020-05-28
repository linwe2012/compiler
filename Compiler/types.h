#ifndef CC_TYPES_H
#define CC_TYPES_H
enum SymbolAttributes
{
	ATTR_NONE,
	ATTR_REGISTER,
	ATTR_AUTO,
	ATTR_EXTERN,
	ATTR_STATIC,
	ATTR_TYPEDEF,

	
	
	ATTR_INLINE = 0x0100,
	ATTR_STDCALL = 0x0200,
	ATTR_CDECL = 0x0400,

	ATTR_MASK_STORAGE = ATTR_INLINE -1,
	
	ATTR_CONST = 0x010000u,
	ATTR_VOLATILE = 0x020000u,
	ATTR_RESTRICT = 0x040000u, //  TODO: this is not supported
};

enum Types
{
	TP_INCOMPLETE = 0u,
	TP_VOID = 1u,
	TP_INT8, // signed char
	TP_INT16, // short
	TP_INT32, // int
	TP_INT64, // long long
	TP_INT128,

	TP_FLOAT32, // float
	TP_FLOAT64, // double
	TP_FLOAT128,

	TP_ELLIPSIS, // ... <- 函数调用的时候

	TP_STRUCT = 0x0010u,
	TP_UNION = 0x0020u,
	TP_ENUM = 0x0040u,
	TP_PTR = 0x0080u, // pointer
	TP_FUNC = 0x0100u, // function

	TP_UNSIGNED = 0x001000u,
	TP_SIGNED = 0x002000u,
	TP_ARRAY = 0x004000u,
	TP_BITFIELD = 0x008000u,
	

	// Type qualifier
	
	TP_LONG_FLAG = 0x080000u,

	TP_CLEAR_SIGNFLAGS = ~(TP_UNSIGNED | TP_SIGNED),
	TP_GET_SIGNFLAGS = (TP_UNSIGNED | TP_SIGNED),
	TP_CLEAR_ATTRIBUTEFLAGS = (TP_UNSIGNED - 1)
};

enum TypesHelper
{
	TP_NUM_BUILTINS = TP_FLOAT128,
	TP_NUM_BUILTIN_INTS = TP_INT128 - 1,
	TP_NUM_BUILTIN_FLOATS = TP_FLOAT128  - TP_FLOAT32 + 1
};

typedef double __float128;
typedef int64_t int128_t;
typedef uint64_t uint128_t;

// typename, typename in c,  bits
#define INTERNAL_TYPE_LIST_MISC(V) \
V(VOID, "void", 8, void, v , "") \
V(ELLIPSIS, "...", 8, ellipse, e, "")

#define INTERNAL_TYPE_LIST_INT(V)\
V(INT8, "char", 8, char, i, "c")\
V(INT16, "short", 16, short, i, "hd")\
V(INT32, "int", 32, int, i, "d") \
V(INT64, "long long", 64, long long, i, "lld")\
V(INT128, "__int128", 128, long long, i, "lld") \



#define INTERNAL_TYPE_LIST_FLOAT(V)\
V(FLOAT32, "float", 32, float, f, "f")\
V(FLOAT64, "double", 64, double, f, "lf")\
V(FLOAT128, "__float128", 128, __float128, f , "lf")

#define INTERNAL_TYPE_LIST_INT_2(V)\
V(INT8, "char", 8, char, i, "c")\
V(INT16, "short", 16, short, i, "hd")\
V(INT32, "int", 32, int, i, "d") \
V(INT64, "long long", 64, long long, i, "lld")\
V(INT128, "__int128", 128, long long, i, "lld") 

#define INTERNAL_TYPE_LIST_FLOAT_2(V)\
V(FLOAT32, "float", 32, float, f, "f")\
V(FLOAT64, "double", 64, double, f, "lf")\
V(FLOAT128, "__float128", 128, __float128, f , "lf")


#define INTERNAL_TYPE_LIST(V)\
INTERNAL_TYPE_LIST_MISC(V)\
INTERNAL_TYPE_LIST_INT(V)\
INTERNAL_TYPE_LIST_FLOAT(V)


union ConstantValue
{
	int8_t i8;
	uint8_t ui8;
	int16_t i16;
	uint16_t ui16;
	int32_t i32;
	uint32_t ui32;
	int64_t i64;
	uint64_t ui64;
	int128_t i128;
	uint128_t ui128;

	float f32;
	double f64;
	__float128 f128;
	char* str;
};



inline int type_numeric_bytes(enum Types t)
{
#define TYPE_BYTES(t, cstr, bits, ...) case TP_##t: return bits / 8;
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

inline int type_is_arithmetic(enum Types t)
{
	t &= TP_CLEAR_SIGNFLAGS;
	return t >= TP_INT8 && t <= TP_FLOAT128;
}

inline enum Types type_integer_promote(enum Types t)
{
	if ((t & TP_CLEAR_SIGNFLAGS) < TP_INT32) return TP_INT32 | (t & TP_GET_SIGNFLAGS);
	return t;
}

// int, int -> int
// int, int64 -> int64
// double, int -> double
// int, float -> float
inline enum Types type_numeric_promote(enum Types t1, enum Types t2)
{
	enum Types t1_ = t1 & TP_CLEAR_ATTRIBUTEFLAGS;
	enum Types t2_ = t2 & TP_CLEAR_ATTRIBUTEFLAGS;


}

#define BINARY_OPERATOR_LIST(V) \
V(MUL, Mul, *) \
V(DIV, Div, /) \
V(MOD, Mod, %)

// referenced here:
// https://www.tutorialspoint.com/cprogramming/c_operators.htm
enum Operators
{
	// Unary Operators
	// ---------------------

	// Both prefix & postfix
	OP_INC,         // ++
	OP_DEC,         // --
	OP_UNARY_STACK_ACCESS,// .

	OP_POSTFIX_INC, // ++
	OP_POSTFIX_DEC, // ++

	// Access
	
	OP_POINTER,     // *
	OP_ADDRESS,     // &identifier


	// Bitwise
	OP_COMPLEMENT,  // ~
	OP_NOT,         // !
	OP_POSITIVE,    // +
	OP_NEGATIVE,    // -

	// Misc
	OP_SIZEOF,      // sizeof
	OP_CAST,        // (type)


	// Arithmetic Operators
	// ---------------------

	// Bitwise
	OP_BIT_AND,     // &
	OP_BIT_OR,      // |
	OP_BIT_XOR,     // ^

	// Multiplicative
	OP_MUL,         // *
	OP_DIV,         // /
	OP_MOD,         // %

	// Additive
	OP_ADD,         // +
	OP_SUB,         // -

	OP_SHIFT_LEFT,  // <<
	OP_SHIFT_RIGHT, // >>

	// Assignment
	// --------------------
	OP_ASSIGN,      // =
	OP_ASSIGN_SHL,  // <<=
	OP_ASSIGN_SHR,  // >>=
	OP_ASSIGN_ADD,  // +=
	OP_ASSIGN_SUB,  // -=
	OP_ASSIGN_MUL,  // *=
	OP_ASSIGN_DIV,  // /=
	OP_ASSIGN_MOD,  // %=
	OP_ASSIGN_AND,  // &=
	OP_ASSIGN_XOR,  // ^=
	OP_ASSIGN_OR,   // |=

	// Logical
	// --------------------
	OP_EQUAL,       // ==
	OP_NOT_EQUAL,   // !=
	OP_LESS,        // <
	OP_LESS_OR_EQUAL,    // <=
	OP_GREATER,          // >
	OP_GREATER_OR_EQUAL, // >=

	OP_AND,         // &&
	OP_OR,          // ||
	OP_STACK_ACCESS, // .
	OP_PTR_ACCESS,   // ->
	OP_ARRAY_ACCESS, // []
	// Misc
	// --------------------
	OP_CONDITIONAL, // ? :

	OP_PREFIX = 0x010000,
	OP_POSTFIX = 0x020000,
};




enum JumpType
{
	JUMP_GOTO,
	JUMP_CONTINUE,
	JUMP_BREAK,
	JUMP_RET,
};

enum LoopType { 
	LOOP_WHILE, 
	LOOP_FOR,
	LOOP_DOWHILE
};


void type_init(struct Context* context);
// static const char* const** (*a)(int a);

#endif // !CC_TYPES_H

