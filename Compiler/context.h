#pragma once
#include "symbol.h"
#include "hashtable.h"

struct Context
{
	// #include "xxx/yyy.h"
	char** include_paths;
	int n_include_paths;

	// #include <xxx.h>
	char** sysinclude_paths;
	int n_sysinclude_paths;

	HashTable* functions;
	HashTable* labels;
	HashTable* varaibles;
	HashTable* types;
};

void enter_scope(Context* ctx);
void leave_scope(Context* ctx);

int sym_add(Context* ctx);
int sym_remove(Context* ctx);
int sym_find(Context* ctx, const char*);