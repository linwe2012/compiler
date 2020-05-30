#include "context.h"

void ctx_string_array_add(char*** p, int* n, char* new_str)
{
	char** new_p = (char**)malloc(sizeof(char*) * (*n + 1));
	memcpy(new_p, *p, sizeof(char*) * (*n));
	free(*p);

	new_p[*n + 1] = new_str;
	*p = new_p;
}

void ctx_enter_block_scope(Context* ctx)
{
	symtbl_enter_scope(ctx->functions, 0);
	symtbl_enter_scope(ctx->variables, 0);
	symtbl_enter_scope(ctx->types, 0);
	symtbl_enter_scope(ctx->labels, 0);
}

void ctx_leave_block_scope(Context* ctx, int free_all)
{
	symtbl_leave_scope(ctx->functions, free_all);
	symtbl_leave_scope(ctx->variables, free_all);
	symtbl_leave_scope(ctx->types, free_all);
	symtbl_leave_scope(ctx->labels, free_all);
}

void ctx_enter_function_scope(Context* ctx) {
	symtbl_enter_scope(ctx->functions, 1);
	symtbl_enter_scope(ctx->variables, 1);
	symtbl_enter_scope(ctx->types, 1);
	symtbl_enter_scope(ctx->labels, 1);
}

void ctx_leave_function_scope(Context* ctx) {
	symtbl_leave_scope(ctx->functions, 1);
	symtbl_leave_scope(ctx->variables, 1);
	symtbl_leave_scope(ctx->types, 1);
	symtbl_leave_scope(ctx->labels, 1);
}
