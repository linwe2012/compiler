#pragma once
#include "symbol.h"
#include "hashtable.h"
#include "types.h"

#include "gen.h"
struct Context
{
	// #include "xxx/yyy.h"
	char** include_paths;
	int n_include_paths;

	// #include <xxx.h>
	char** sysinclude_paths;
	int n_sysinclude_paths;

	struct SymbolTable* functions;
	struct SymbolTable* labels;
	struct SymbolTable* variables; // enum & ids
	struct SymbolTable* types;
	struct SymbolTable* enums;

	struct Gen* gen;
	
	struct AST* current;
	void* ast_data; // 给 AST 用的数据
	struct {
		int allow_duplicate_typedef;
	} options;
};
STRUCT_TYPE(Context)

void ctx_enter_block_scope(Context* ctx);
void ctx_leave_block_scope(Context* ctx, int free_all_symbols);
void ctx_enter_function_scope(Context* ctx);
void ctx_leave_function_scope(Context* ctx);
