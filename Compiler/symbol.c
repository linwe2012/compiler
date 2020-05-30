#include "symbol.h"
#include "context.h"
#include "error.h"
#include "utils.h"


STRUCT_TYPE(SymbolTableEntry);

TypeInfo builtins[TP_NUM_BUILTINS + TP_NUM_BUILTIN_INTS + 2];
#define ERROR_TYPE_INDEX TP_NUM_BUILTINS + TP_NUM_BUILTIN_INTS + 1

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif // !max

Symbol* new_symbol(char* name, enum SymbolTypes type);

void symtbl_init(SymbolTable* tbl)
{

	NEW_STRUCT(SymbolStackInfo, global);
	memset(global, 0, sizeof(global));

	tbl->bottom = tbl->stack_top = global;
}

SymbolTable* symtbl_new()
{
	NEW_STRUCT(SymbolTable, tbl);
	symtbl_init(tbl);
	return tbl;
}

void symtbl_push(SymbolTable* tbl, Symbol* c)
{
	// 如果是这个 scope 的第一个元素
	if (tbl->stack_top->first == NULL)
	{
		tbl->stack_top->first = c;
		if (tbl->stack_top->prev)
		{
			c->prev = tbl->stack_top->prev->last;
		}
		// 如果是整个文件的第一个元素
		else {
			c->prev = NULL;
		}
	}
	else {
		c->prev = tbl->stack_top->last;
		tbl->stack_top->last = c;
	}
	
}

Symbol* symtbl_find(SymbolTable* tbl, const char* name)
{
	Symbol* top = tbl->stack_top->last;
	while (top != NULL)
	{
		while (!str_equal(top->name, name))
		{
			top->prev;
		}
	}

	return top;
}


void symtbl_enter_scope(SymbolTable* tbl, int keep_global_only)
{
	NEW_STRUCT(SymbolStackInfo, entry);
	if (keep_global_only)
	{
		entry->prev = tbl->bottom;
	}
	else{
		entry->prev = tbl->stack_top;
	}
	entry->real_prev = tbl->stack_top;
	tbl->stack_top->next = entry;
	entry->next = NULL;
	entry->nest = tbl->stack_top->nest + 1;
	tbl->stack_top = entry;
}

void symtbl_leave_scope(SymbolTable* tbl, int free_all_symols)
{
	struct SymbolStackInfo* top = tbl->stack_top;
	Symbol* cur = top->last;
	Symbol* prev;
	if (free_all_symols)
	{
		while (cur != top->first)
		{
			prev = cur->prev;
			free(cur);
			cur = prev;
		}
		free(cur);
	}

	tbl->stack_top = top->real_prev;
	free(top);
}



/*
Symbol* sym_find(Context* ctx, enum SymbolTypes type, const char* str)
{
	SymbolTable* tbl;
	switch (type)
	{
	case Symbol_TypeInfo:
		tbl = ctx->types;
		break;
	case Symbol_VariableInfo:
		tbl = ctx->variables;
		break;
	case Symbol_FunctionInfo:
		tbl = ctx->functions;
		break;
	default:
		log_internal_error(ctx->current, "Unexpected symbol type %d", type);
		return NULL;
	}

	SymbolTableEntry* entry = tbl->stack_top;
	while (entry)
	{
		Symbol* sym = hash_find(entry->table, str);
		if (sym)
		{
			return sym;
		}

		if (entry->hide_bottom)
		{
			break;
		}
	}

	return hash_find(tbl->global->table, str);
}

Symbol* sym_add_type()
{

}

void sym_init_type_symbol()
{

}

*/


//TODO
/*
void sym_make_alias(Context* ctx, Symbol* sym, const char* alias)
{
	CC_ASSERT(sym->usage == Symbol_TypeInfo, ctx->current, "Wrong symbol type");
	TypeInfo* type = &sym->type;
	if (type->is_alias)
	{
		// type = ((TypeAlias*)type)->info;
	}
	Symbol* declared = sym_find(ctx, Symbol_TypeInfo, alias);
	if (declared)
	{
		if (ctx->options.allow_duplicate_typedef)
		{
			if (declared->type.is_alias &&  )
			{

			}
		}
	}

	struct TypeAlias* entry = sym->type.alias;
	while (entry)
	{
		if (str_equal(entry->alias, alias))
		{

		}
	}
	NEW_STRUCT(TypeAlias, type_alias);
	type_alias->alias;
	type_alias->is_alias = 1;

	type_alias->info = sym;
	type_alias->next = NULL;
	type_alias->prev = NULL;
	


}*/

void type_info_init(TypeInfo* info)
{
	memset(info, 0, sizeof(TypeInfo));
	info->alignment = -1;
	info->aligned_size = -1;
	info->offset = -1;
	info->bitfield = -1;
	info->bitfield_offset = -1;
	info->prev = NULL;
	info->next = NULL;
	info->incomplete = 0;
}

TypeInfo* type_create_array(uint64_t n, enum SymbolAttributes qualifers, TypeInfo* array_element_type)
{
	Symbol* sym = new_symbol(NULL, Symbol_TypeInfo);
	TypeInfo* info = &sym->type;
	type_info_init(info);

	info->type = TP_ARRAY;
	info->arr.array_count = n;
	info->qualifiers = qualifers;
	info->arr.array_type = array_element_type;

	return info;
}

TypeInfo* type_create_struct_or_union(enum Types type, char* name)
{
	NEW_STRUCT(TypeInfo, info);
	type_info_init(info);

	info->type = type;
	info->type_name = name;
	return info;
}

TypeInfo* type_create_ptr(enum SymbolAttributes qualifers, struct TypeInfo* pointing)
{
	Symbol* sym = new_symbol(NULL, Symbol_TypeInfo);
	TypeInfo* info = &sym->type;
	type_info_init(info);


	info->qualifiers = qualifers;
	info->type = TP_PTR;
	info->alignment = 8;
	info->aligned_size = 8;
	info->ptr.pointing = pointing;

	return info;
}

TypeInfo* type_create_func(struct TypeInfo* ret, char* name, struct TypeInfo* params)
{
	Symbol* sym = new_symbol(NULL, Symbol_TypeInfo);
	TypeInfo* info = &sym->type;
	type_info_init(info);

	//TODO: should i add alignment?
	info->type = TP_FUNC;
	info->fn.params = params;
	info->fn.return_type = ret;

	return info;
}



TypeInfo* create_struct_field(TypeInfo* type_info, enum SymbolAttributes attributes, char* field_name)
{
	type_info->struc_field.attrib = attributes;
	type_info->field_name = field_name;
	return type_info;
}

int type_wrap(TypeInfo* parent, TypeInfo* child)
{
	if (parent->type == TP_FUNC || TP_ARRAY || TP_STRUCT || TP_PTR)
	{
		parent->struc.child = child;
		return 0;
	}

	return 1;
}

TypeInfo* type_get_child(TypeInfo* parent)
{
	if (parent->type == TP_FUNC || TP_ARRAY || TP_STRUCT || TP_PTR)
	{
		return parent->struc.child;
	}

	return NULL;
}

void symbol_init_context(struct Context* context)
{
	enum Types theor = TP_INCOMPLETE;
	enum TypesHelper offset = 0;
	const char* prefix = "";
#define INIT(type__, name__, bits__, ...){ \
	struct TypeInfo type;\
	type_info_init(&type);\
	type.aligned_size = bits__ / 8;\
	type.alignment = bits__ / 8;\
\
	type.type_name = str_concat(prefix, name__);\
	type.type = theor | TP_##type__;\
\
	builtins[offset + TP_##type__] = type;}

	INTERNAL_TYPE_LIST_MISC(INIT);
	INTERNAL_TYPE_LIST_INT(INIT);
	INTERNAL_TYPE_LIST_FLOAT(INIT);

	theor = TP_UNSIGNED;
	offset = TP_NUM_BUILTINS + 1;
	prefix = "unsigned ";

	INTERNAL_TYPE_LIST_INT(INIT);

	{
		struct TypeInfo type;
		type_info_init(&type);
		type.type_name = "<error type>";
		type.type = TP_ERROR;
		builtins[ERROR_TYPE_INDEX] = type;
	}

}


TypeInfo* type_fetch_buildtin(enum Types type)
{
	int is_unsign = (type & TP_SIGNED);
	int inc = is_unsign ? TP_NUM_BUILTINS : 0;

	switch (type & TP_CLEAR_SIGNFLAGS)
	{
	case TP_INCOMPLETE:
		break;
	case TP_VOID:  // fall through
	case TP_INT8:
	case TP_INT16:
	case TP_INT32:
	case TP_INT64:
	case TP_INT128:
		return &builtins[inc + type & TP_CLEAR_SIGNFLAGS];
	case TP_FLOAT32:
	case TP_FLOAT64:
	case TP_FLOAT128:
		return &builtins[type & TP_CLEAR_SIGNFLAGS];
	default:
		break;
	}

	return NULL;
}

Symbol* new_symbol(char* name, enum SymbolTypes type)
{
	NEW_STRUCT(Symbol, sym);
	memset(sym, 0, sizeof(Symbol));

	sym->name = name;
	sym->usage = type;

	return sym;
}


Symbol* symbol_create_label(char* name, uint64_t label, int resolved)
{
	Symbol* sym = new_symbol(name, Symbol_LabelInfo);

	sym->label.label_id = label;
	sym->label.resolved = resolved;

	return sym;
}

Symbol* symbol_create_constant(Symbol* enum_sym, char* name, void* val)
{
	Symbol* sym = new_symbol(name, Symbol_VariableInfo);
	sym->var.attributes = ATTR_NONE;
	sym->var.value = val;
	sym->var.is_constant = 1;

	sym->var.type = enum_sym;

	return sym;
;}

Symbol* symbol_create_enum(char* name)
{
	Symbol* sym = new_symbol(name, Symbol_TypeInfo);
	type_info_init(&(sym->type));

	sym->type.type = TP_ENUM;
	sym->type.alignment = 8;
	sym->type.aligned_size = 8;
	
	return sym;
}

Symbol* symbol_create_enum_item(Symbol* type, Symbol* prev, char* name, void* val)
{
	Symbol* sym = new_symbol(name, Symbol_TypeInfo);
	sym->var.name = name;
	sym->var.attributes = ATTR_CONST;
	sym->var.is_constant = 1;
	sym->var.value = val;
	sym->var.type = type;
	sym->var.prev = &prev->var;

	return sym;
}

void variable_append(Symbol* last, Symbol* new_last)
{
	if (last == NULL) return;
	last->var.next = &new_last->var;
	new_last->var.prev = &last->var;
}


Symbol* symbol_from_type_info(TypeInfo* info)
{
	Symbol* sym = new_symbol(info->type_name, Symbol_TypeInfo);
	sym->type = *info;
	if (info->next)
	{
		info->next->prev = &(sym->type);
	}
	return sym;
}

union ConstantValue constant_cast(enum Types from, enum Types to, union ConstantValue src)
{
	union ConstantValue res;
	int error = 0;

	from &= ~TP_SIGNED;
	to &= ~TP_SIGNED;
	
#define INNER_SWICTH(type, name, bits, type_name, prefix, format)\
	case TP_##type: res.prefix##bits = data; break;

#define INNER_SWICTH_UNSIGNED(type, name, bits, type_name, prefix, format)\
	case TP_##type | TP_UNSIGNED: res.prefix##bits = data; break;

#define INNER_CASES \
	INTERNAL_TYPE_LIST_INT_2(INNER_SWICTH) \
	INTERNAL_TYPE_LIST_INT_2(INNER_SWICTH_UNSIGNED) \
	INTERNAL_TYPE_LIST_FLOAT_2(INNER_SWICTH) \

#define OUTER_SWICTH(type, name, bits, type_name, prefix, format)\
	case TP_##type: \
	{   type_name data = src.prefix##bits; \
		switch (to){\
		INNER_CASES \
		default:\
		error = 1;\
		break;\
		}\
	}

#define OUTER_SWICTH_UNSIGNED(type, name, bits, type_name, prefix, format)\
	case TP_##type | TP_UNSIGNED: \
	{   unsigned type_name data = src.u##prefix##bits; \
		switch (to){\
		INNER_CASES \
		default:\
		error = 1;\
		break;\
		}\
	}

	switch (from){

		INTERNAL_TYPE_LIST_INT(OUTER_SWICTH)
		INTERNAL_TYPE_LIST_INT(OUTER_SWICTH_UNSIGNED)
		INTERNAL_TYPE_LIST_FLOAT(OUTER_SWICTH)
		default:
			error = 1;
			break;
	}
	return res;
}


//TODO: Double check 我算的 对不对
//TODO: 支持 bit field

Symbol* symbol_create_struct_or_union(TypeInfo* info, TypeInfo* child)
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

	return symbol_from_type_info(info);
}
TypeInfo* type_get_error_type()
{
	return &builtins[ERROR_TYPE_INDEX];
}

Symbol* symbol_create_struct_or_union_incomplete(char* name, enum Types struct_or_union)
{
	Symbol* sym = new_symbol(name, Symbol_TypeInfo);
	TypeInfo* info = &sym->type;
	type_info_init(info);

	info->type = struct_or_union;
	info->incomplete = 1;

	return sym;
}

Symbol* symbol_create_func(char* name, void* val, TypeInfo* ret, TypeInfo* params, AST* body) {
	Symbol* sym = new_symbol(name, Symbol_FunctionInfo);
	sym->func.name = name;
	sym->func.body = body;
	sym->func.params = params;
	sym->func.return_type = ret;
	sym->func.value = val;		// 这个值是LLVMValueRef吧?
	return sym;
}

TypeInfo* type_create_param_ellipse()
{
	NEW_STRUCT(TypeInfo, info);
	type_info_init(info);
	info->type = TP_ELLIPSIS;
	return info;
}

int type_append(TypeInfo* tail, TypeInfo* new_tail)
{
	tail->next = new_tail;
	new_tail->prev = tail;
	return 0;
}

void value_constant_print(FILE* f, enum Types type, union ConstantValue* pval)
{
#define PRT_CASE(type_name, pretty_name, bits, c_type, prefix, formater) \
	case TP_##type_name: fprintf(f, "(" pretty_name ")" "%" formater, pval->prefix##bits); break;

#define PRT_CASE_UNSIGNED(type_name, pretty_name, bits, c_type, prefix, formater) \
	case TP_##type_name | TP_UNSIGNED: \
	fprintf(f, "(" pretty_name ")" "%" formater "u" , pval->u##prefix##bits); break;

	switch (type)
	{
		INTERNAL_TYPE_LIST_FLOAT(PRT_CASE);
		INTERNAL_TYPE_LIST_INT(PRT_CASE);
		INTERNAL_TYPE_LIST_INT(PRT_CASE_UNSIGNED);

	default:
		break;
	}

#undef PRT_CASE
#undef PRT_CASE_UNSIGNED
}