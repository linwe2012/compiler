#include "symbol.h"
#include "context.h"
#include "error.h"
#include "utils.h"

STRUCT_TYPE(SymbolTableEntry);

void sym_type_init(SymbolTable* tbl)
{

	NEW_STRUCT(SymbolTableEntry, entry);
	entry->table = hash_new_strkey(sizeof(struct SymbolTableEntry), 9);

	tbl->stack_bottom = tbl->stack_top = entry;
}

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