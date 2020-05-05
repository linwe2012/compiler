#include "ast.h"
#include "error.h"
#include <assert.h>
#define NEW_AST(type, name) \
	type* name = (type*) malloc (sizeof(type)); \
	ast_init(&name->super, AST_##type);

#define SUPER(ptr) &(ptr->super)

// TODO add check
#define CAST(type, name, from) type* name =  (type*) from;


void ast_init(AST* ast, ASTType type)
{
	ast->prev = NULL;
	ast->next = NULL;

	ast->type = type;
}

int check_ast(AST* ast)
{
	return ast->type >= AST_EmptyExpr && ast->type <= AST_FunctionExpr;
}

AST* ast_append(AST* leader, AST* follower)
{
	// assert()
	leader->next = follower;
	follower->prev = leader;
	return leader;
}

AST* make_number_int32(const char* c)
{
	NEW_AST(NumberExpr, ast);
	ast->number_type = TP_INT32;
	ast->i32 = atoi(c);
	return SUPER(ast);
}

AST* make_number_float(const char* c, int bits)
{
	NEW_AST(NumberExpr, ast);
	if (bits == 32)
	{
		ast->number_type = TP_FLOAT32;
		ast->f32 = (float)atof(c);
	}
	else {
		ast->number_type = TP_FLOAT64;
		ast->f64 = atof(c);
	}
	return SUPER(ast);
}

AST* make_identifier(const char* c)
{
	log_error(NULL, "id, %s", c);
	// printf("No %s");
	//TODO
	return NULL;
}

Symbol* sym_get_type(const char* n)
{
	//TODO
	Symbol* sym = (Symbol*)malloc(sizeof(Symbol));
	sym->name = n;
	return sym;
}

Symbol* sym_get_identifier(const char* n)
{
	//TODO
	return sym_get_type(n);
}

FunctionDefinition* sym_get_function(const char* return_type, const char* name, AST* args)
{
	FunctionDefinition* def = (FunctionDefinition*)malloc(sizeof(FunctionDefinition));
	def->body = NULL;
	def->name = name;
	def->params = args;
	//TODO
	return def;
}

AST* make_symbol(const char* type, const char* name)
{
	NEW_AST(SymbolExpr, ast);

	ast->type = sym_get_type(type);
	ast->name = sym_get_identifier(name);

	return SUPER(ast);
}

AST* make_block(AST* first_child)
{
	NEW_AST(BlockExpr, ast);

	ast->first_child = first_child;

	return SUPER(ast);
}

AST* make_function_call(const char* function_name, AST* params)
{
	NEW_AST(FunctionCallExpr, ast);

	ast->function_name = function_name;
	ast->params = params;

	return SUPER(ast);
}

AST* make_function(const char* return_type, const char* name, AST* arg_list)
{

	NEW_AST(FunctionExpr, func);

	func->ref = sym_get_function(return_type, name, arg_list);

	return SUPER(func);
}

AST* make_function_body(AST* ast, AST* body)
{
	CAST(FunctionExpr, func, ast);
	func->ref->body = body;

	return ast;
}

AST* make_return(AST* exp)
{
	NEW_AST(ReturnStatement, ast);
	ast->return_val = exp;

	return SUPER(ast);
}

AST* make_empty()
{
	NEW_AST(EmptyExpr, ast);
	return SUPER(ast);
}