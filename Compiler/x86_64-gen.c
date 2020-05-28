/*
#include <windows.h>
#include <winnt.h>
#include <time.h>
#include <stdint.h>

#include <stdio.h>
#include "x86_64-asm.h"
#include "error.h"
#include "ast.h"

// Register b##R;

#define DEFINE_REGISTER(R) \
Value* R;


GENERAL_REGISTERS(DEFINE_REGISTER)
DOUBLE_REGISTERS(DEFINE_REGISTER)
I386_REGISTERS(DEFINE_REGISTER)
X86_REGISTERS(DEFINE_REGISTER)
#undef DEFINE_REGISTER

Register registers[kNumRegisterCodes];

// Linux Definition of Coff: http://www.delorie.com/djgpp/doc/coff/filhdr.html

#define SECTION_DATA ".data"
#define SECTION_BSS  ".bss"

// 初始化全局变量 寄存器
void x64_init_registers()
{
#define INIT_REGISTERS(R) \
	registers[kRegCode_##R].code = kRegCode_##R;  \
	registers[kRegCode_##R].super.type = Value_Register; \
	registers[kRegCode_##R].name = #R; \
	registers[kRegCode_##R].mode = current_mode; \
	R = &(registers[kRegCode_##R].super); \
	

	enum RegisterPart current_mode = kReg64;
	GENERAL_REGISTERS(INIT_REGISTERS);

	current_mode = kRegXMM;
	DOUBLE_REGISTERS(INIT_REGISTERS);

	current_mode = kReg16;
	I386_REGISTERS(INIT_REGISTERS);

	current_mode = kReg32;
	X86_REGISTERS(INIT_REGISTERS);

#undef INIT_REGISTERS
}

void x64_asm_init(ASMContext* ctx)
{
	x64_init_registers();
}



char int2hex(unsigned int x)
{
	CC_ASSERT(x >= 0 && x <= 16, NULL, "int2hex");
	if (x < 10)
	{
		return '0' + x;
	}
	return 'a' + (x-10);
}

int host_is_big_endian()
{
	union {
		uint32_t i;
		char c[4];
	} e = { 0x01000000 };

	return e.c[0];
}

char* get_hex_string(const void* val, int bytes)
{
	char* s = malloc(bytes * 2+1);
	const unsigned char* u = val;
	int i = bytes;
	for (int x = 0; x < bytes; ++x)
	{
		--i;
		s[x*2] = int2hex((u[i] & 0xF0) >> 4);
		s[x*2+1] = int2hex(u[i] & 0xF);
	}
	s[bytes * 2] = '\0';
	return s;
}

enum RegisterMode get_regitser_mode_by_type(enum Types type)
{
	if (type & TP_PTR || type & TP_FUNC)
	{
		return kReg64;
	}
	if (type & TP_ARRAY || type & TP_STRUCT || type & TP_UNION)
	{
		return kRegMemory;
	}

	// 清除 enum
	type &= (~TP_ENUM);
	type &= TP_CLEAR_SIGNFLAGS;

	switch (type & TP_CLEAR_ATTRIBUTEFLAGS)
	{
	case TP_VOID:
		return kReg16;
	case TP_INT8: // fall through
	case TP_INT16:
		return kReg16;
	case TP_INT32:
		return kReg32;
	case TP_INT64:
		return kReg64;
	case TP_INT128:
		return kRegMemory;
	case TP_FLOAT32: // fall through
	case TP_FLOAT64:
		return kRegXMM;
	case TP_FLOAT128:
		return kRegMemory;
	default:
		break;
	}

	return kRegMemory;
}

Value* free_val(Value* v)
{
	if (v == NULL)
	{
		return NULL;
	}
	if (v->type == Value_Register)
	{
		return NULL;
	}
	free(v);
	return NULL;
}

char* format_str(const char* fmt, ...)
{
	char* c = malloc(256);
	int size = 256;
	va_list args;
	va_start(args, fmt);

	while (1)
	{
		int cx = vsnprintf(c, size, fmt, args);
		if (cx >= 0 && cx < size)
		{
			return c;
		}
		else if (size > 1024 * 1024 * 128)
		{
			log_error(NULL, "unable to format string");
			strcpy(c, "<error>");
			return c;
		}
		else
		{
			size *= 2;
			free(c);
			c = malloc(size);
		}
	}
	
	
	va_end(args);
}

IMAGE_FILE_HEADER get_empty_coff()
{
	IMAGE_FILE_HEADER coff = {
		// COFF文件属于何种平台.
		.Machine =  IMAGE_FILE_MACHINE_AMD64,

		// 记录COFF一共有多少个区段.在COFF文件中,根据符号的类型不同被划分到不同的区域中.
		.NumberOfSections = 0,

		// COFF文件被创建的时间.
		.TimeDateStamp = time(NULL),

		// 符号表在文件中的偏移.
		.PointerToSymbolTable = 0,

		// 一个COFF文件中符号的总个数.
		.NumberOfSymbols = 0,

		// COFF文件中都没有可选头,因此这个字段的值是0
		.SizeOfOptionalHeader = 0,

		// 此字段记录着文件的属性
		.Characteristics = 
			// 用户层空间可大于2GB, https://stackoverflow.com/questions/586826/image-file-large-address-aware-and-3gb-os-switch
			// stackoverflow 上提到 64 位这个 bit 默认是 set 的
			IMAGE_FILE_LARGE_ADDRESS_AWARE 

	};

	return coff;
}


IMAGE_SECTION_HEADER header;

IMAGE_SECTION_HEADER get_coff_section(const char* name, uint32_t character)
{
	// name 最大为8个字节的,以’\0’为结尾的ASCII字符串.用于记录区段的名字.
	// 区段的名字有些是特定意义的区段. 如果区段名的数量大于8个字节,
	// 则name的第一字节是一个斜杠字符:’/’,接着就是一个数字,这个数字就是字符串表的一个索引.
	// 它将索引到一个具体的区段名.
	// TODO： 支持 >8 字节的区段

	unsigned char Name[8];
	strcpy(Name, name);

	IMAGE_SECTION_HEADER header = {
		.Name = Name,

		// 在COFF文件中都没有作用,都是0.
		.Misc.VirtualSize = 0,
		.VirtualAddress = 0,

		// 这个字段记录区段的原始数据的字节数.
		.SizeOfRawData = 0,

		// 区段原始数据在文件中的偏移.
		.PointerToRawData = 0,

		// 区段重定位表偏移
		.PointerToRelocations = 0,

		// 行号表的文件偏移
		.PointerToLinenumbers = 0,

		// 重定位表条目个数
		.NumberOfRelocations = 0,

		// 行号表个数
		.NumberOfLinenumbers = 0,

		// 段标识
		.Characteristics = character
	};

	return header;
}





void write_str(ASMContext* ctx, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	printf("\n");
}

#define DECL_VISITOR(x) Value* x64_visit_##x(ASMContext* ctx,  x* ast);
AST_NODE_LIST(DECL_VISITOR)
#undef DECL_VISITOR

Value* x64_visit_ast(ASMContext* ctx, AST* ast)
{

#define AST_VISTOR_CASE(x) case AST_##x: return x64_visit_##x(ctx, (x*)ast);
	switch (ast->type)
	{
		AST_NODE_LIST(AST_VISTOR_CASE)
	default:
		break;
	}
#undef AST_VISTOR_CASE

	log_internal_error(ast, "Internal error, unkown ast type");
	return NULL;
}

Value* x64_visit_EmptyExpr(ASMContext* ctx,  EmptyExpr* ast)
{
	return NULL;
}

Value* x64_visit_BlockExpr(ASMContext* ctx, BlockExpr* ast)
{
	//TODO: handle symbol life cycle

	AST* child = ast->first_child;

	while (child)
	{
		free_val(x64_visit_ast(ctx, child));

		child = child->next;
	}

	return NULL;
}


Value* x64_visit_ReturnStatement(ASMContext* ctx, ReturnStatement* ast)
{
	Value* val = x64_visit_ast(ctx, ast->return_val);
	emit_mov(ctx, rax, val);
	//TODO
	emit_mov(ctx, rsp, rbp);
	emit_pop(ctx, rbp);
	emit_ret(ctx, 0);

	return free_val(val);
}

Value* x64_visit_ListExpr(ASMContext* ctx, ListExpr* ast)
{
	Value* val = NULL;
	AST* child = ast->first_child;

	while (child)
	{
		free_val(val);
		val = x64_visit_ast(ctx, child);

		child = child->next;
	}
	return val;
}

// 根据 src 的类型挑选寄存器
Register* x64_pick_register_by_value(ASMContext* ctx,
	enum RegisterCode general, enum RegisterCode float_point, 
	Value* src)
{
	struct TypedValueInterface* typed;
	enum RegisterMode mode;
	Register* picked;

	int pick_by_mode[] = {
		kRegCode_ax, // kReg16
		kRegCode_eax,  // kReg32
		0,                  // kReg64
		kRegCode_xmm0, // kRegXMM,
		0,                   // kRegMemory
	};

	if (src->type == Value_Memory || src->type == Value_Immediate)
	{
		typed = (struct TypedValueInterface*) src;
		mode = get_regitser_mode_by_type(typed->type);
	}

	else {
		Register* reg = (Register*)src;
		mode = reg->mode;
	}

	if (mode == kRegXMM)
	{
		picked = &registers[float_point];
	}
	else {
		picked = &registers[pick_by_mode[mode] + general];
	}
	return picked;
}

Value* x64_visit_FunctionCallExpr(ASMContext* ctx, FunctionCallExpr* ast)
{
	AST* param = ast->params;
	int n_params = 1;
	while (param && param->next)
	{
		++n_params;
		param = param->next;
	}

	int64_t bytes = 0;

	while (param)
	{
		--n_params;
		Value* val = x64_visit_ast(ctx, param);

		if (n_params > 3)
		{
			bytes += emit_push(ctx, val);
		}
		
		else {
			Register* picked = NULL;
			if (n_params == 0)
			{
				picked = x64_pick_register_by_value(ctx, kRegCode_rcx, kRegCode_xmm0, val);
			}
			else if (n_params == 1)
			{
				picked = x64_pick_register_by_value(ctx, kRegCode_rdx, kRegCode_xmm1, val);
			}
			else if (n_params == 2)
			{
				picked = x64_pick_register_by_value(ctx, kRegCode_r8, kRegCode_xmm2, val);
			}
			else if (n_params == 3)
			{
				picked = x64_pick_register_by_value(ctx, kRegCode_r9, kRegCode_xmm3, val);
			}
			if (picked != NULL)
			{
				emit_mov(ctx, picked, val);
			}
			else {
				log_internal_error(NULL, "Unable to pick a register for function parameters");
			}
		}
		free_val(val);
		param = param->prev;
	}

	char* func = format_str("_%s", ast->function_name);
	emit_call(ctx, func);
	free(func);
	Value* sub = make_immediate_num(&bytes, TP_INT64);
	emit_sub(ctx, rsp, sub);
	free_val(sub);

	//TODO function call value
	return NULL;
}

Value* x64_visit_SymbolExpr(ASMContext* ctx, SymbolExpr* ast)
{
	//TODO
	return NULL;
}

Value* x64_visit_FunctionExpr(ASMContext* ctx, FunctionExpr* ast)
{
	if (ast->ref->body)
	{
		char* func = format_str("_%s", ast->ref->name);
		emit_label(ctx, func);
		free(func);

		AST* body = ast->ref->body;
		//TODO handle params
		while (body)
		{
			x64_visit_ast(ctx, body);
			body = body->next;
		}
	}

	return NULL;
}

Value* x64_visit_NumberExpr(ASMContext* ctx, NumberExpr* num)
{
	return make_immediate_num(&num->i32, num->number_type);
}

Value* make_immediate_num(void* x, enum Types type)
{
	Immediate* im = malloc(sizeof(Immediate));
	im->super.type = Value_Immediate;
	im->type = type;
	memcpy(&im->i32, x, type_numeric_bytes(type));
	return &im->super;
}



char* value_str(Value* src)
{
	if (src->type == Value_Immediate)
	{
		Immediate* im = (Immediate*)src;
		if (im->type == TP_INT64)
		{
			return format_str("qword %ld", im->i64);
		}
		if (im->type == TP_INT32)
		{
			return format_str("dword %ld", im->i32);
		}
		if (im->type == TP_FLOAT32)
		{
			char* r = get_hex_string(&im->f32, 4);
			char* m = format_str("dword ptr __real@%s", r);
			free(r);
			return m;
		}
		if (im->type == TP_FLOAT64)
		{
			char* r = get_hex_string(&im->f64, 8);
			char* m = format_str("qword ptr __real@%s", r);
			free(r);
			return m;
		}
	}
	else if (src->type == Value_Register)
	{
		Register* reg = (Register*)src;
		return format_str("%s", reg->name);
	}
}

void emit_mov(ASMContext* ctx, Value* dst, Value* src)
{
	if (ctx->assembly)
	{
		char* mov = "mov";
		if (dst->type == Value_Register)
		{
			Register* reg = (Register*)dst;
			if (src->type == Value_Immediate)
			{
				Immediate* im = (Immediate*)src;
				if (im->type == TP_FLOAT32)
				{
					if (reg->mode == kRegXMM)
					{
						mov = "movss";
					}
				}
				else if (im->type == TP_FLOAT64)
				{
					if (reg->mode == kRegXMM)
					{
						mov = "movsd";
					}
				}
			}
			
		}
		char* s_src = value_str(src);
		char* s_dst = value_str(dst);
		
		write_str(ctx, "%s %s, %s", mov, s_dst, s_src);
		free(s_src);
		free(s_dst);
		
	}
}

void emit_pop(ASMContext* ctx, Value* reg)
{
	if (ctx->assembly)
	{
		Register* r = (Register*)reg;
		write_str(ctx, "pop %s", r->name);
	}


	
}

void emit_ret(ASMContext* ctx, int pop)
{
	if (ctx->assembly)
	{
		write_str(ctx, "ret %d", pop);
	}
}

int value_size(Value* val)
{
	if (val->type == Value_Immediate)
	{
		Immediate* im = (Immediate*)val;
		return type_numeric_bytes(im->type);
	}
	// TODO
	return -1;
}

int emit_push(ASMContext* ctx, Value* val)
{
	if (ctx->assembly)
	{
		if (val->type == Value_Immediate)
		{
			Immediate* im = (Immediate*)val;
			char* v = value_str(val);
			write_str(ctx, "push %s", v);
			free(v);
		}


	}

	return value_size(val);
	

}

// dst -= src
void emit_sub(ASMContext* ctx, Value* dst, Value* src)
{
	if (ctx->assembly)
	{
		char* s_dst = value_str(dst);
		char* s_src = value_str(src);
		write_str(ctx, "sub %s, %s", s_dst, s_src);

		free(s_dst);
		free(s_src);
	}
}

void emit_call(ASMContext* ctx, const char* name)
{
	if (ctx->assembly)
	{
		write_str(ctx, "call %s", name);
	}
}

void emit_label(ASMContext* ctx, const char* name)
{
	if (ctx->assembly)
	{
		write_str(ctx, "%s:", name);
	}
}
*/