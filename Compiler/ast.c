#include "ast.h"
#include "error.h"
#include <assert.h>
#include "types.h"
#include "context.h"

#define NEW_AST(type, name) \
	type* name = (type*) malloc (sizeof(type)); \
	ast_init(&name->super, AST_##type);

#define SUPER(ptr) &(ptr->super)

#define SET_TYPE(ptr, type__) ptr->super.type = type__

// (ast*)from -> (type*) name
// 会执行类型检查
#define CAST(type__, name__, from__) \
	if(from__->type != AST_##type__) \
	{ \
		return make_error("Expected " #type__ " when receiving" ); \
	} \
	type__* name__ =  (type__*) from__

#define FOR_EACH(ast__, iterator__) \
	for(AST* iterator__ = ast__; iterator__ != NULL; iterator__ = iterator__->next)


Context* ctx;


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

AST* make_number_int32(char* c)
{
	NEW_AST(NumberExpr, ast);
	ast->number_type = TP_INT32;
	ast->i32 = atoi(c);

	free(c);
	return SUPER(ast);
}

AST* make_string(char* c)
{
	NEW_AST(NumberExpr, ast);
	ast->number_type = TP_INT32;
	ast->str = c;

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
	ast->error = NULL;
	return SUPER(ast);
}

AST* make_error(char* message)
{
	NEW_AST(EmptyExpr, ast);
	ast->error = message;
	return SUPER(ast);
}

void* ast_destroy(AST* rhs)
{

}

AST* make_unary_expr(enum Operators unary_op, AST* rhs)
{
	NEW_AST(OperatorExpr, ast);
	enum Types type = rhs->sym_type->type.type;
	const char* err_msg = NULL;
	AST* other;

	switch (unary_op)
	{
	case OP_INC:
		break;
	case OP_DEC:
		break;
	case OP_UNARY_STACK_ACCESS:
		break;
	case OP_POSTFIX_INC:
		break;
	case OP_POSTFIX_DEC:
		break;
	case OP_POINTER:
		break;
	case OP_ADDRESS:
		break;
	case OP_COMPLEMENT:
		break;
	case OP_NOT:
		break;
	case OP_POSITIVE: // fall through
	case OP_NEGATIVE:
		if (type_is_arithmetic(type))
		{
			if (type_is_float_point(type))
			{
				SET_TYPE(ast, type);
			}
			else {
				SET_TYPE(ast, type_integer_promote(type));
			}
		}
		else {
			err_msg = "Expected alrithmetic type";
		}
		break;
	case OP_SIZEOF:
	{
		if (type == TP_INCOMPLETE)
		{
			err_msg = "sizeof operator shall not be applied to an incomplete type";
		}
		else if (type == TP_FUNC)
		{
			err_msg = "sizeof operator shall not be applied to function type";
		}
		else if (type & TP_BITFIELD)
		{
			err_msg = "sizeof operator shall not be applied to a bit-ﬁeld member";
		}
		else {
			NEW_AST(NumberExpr, num);
			num->super.sym_type = &SymbolType_UINT64;
			num->number_type = TP_INT64 | TP_UNSIGNED;
			num->ui64 = rhs->sym_type->type.aligned_size;
			other = SUPER(num); 
		}
		
		break;
	}
	case OP_CAST:
		break;
	
	default:
		break;
	}

	if (err_msg)
	{
		ast_destroy(rhs);
		ast_destroy(ast);
		return make_error(err_msg);
	}

	if (other)
	{
		ast_destroy(rhs);
		ast_destroy(ast);
		return other;
	}

	ast->op = unary_op;
	ast->rhs = rhs;
	
	return SUPER(ast);
}

AST* make_binary_expr(enum Operators binary_op, AST* lhs, AST* rhs)
{
	NEW_AST(OperatorExpr, ast);

	ast->op = binary_op;
	ast->rhs = rhs;
	ast->lhs = lhs;

	return SUPER(ast);
}


AST* make_trinary_expr(enum Operators triary_op, AST* cond, AST* lhs, AST* rhs)
{
	NEW_AST(OperatorExpr, ast);

	ast->op = triary_op;
	ast->rhs = rhs;
	ast->lhs = lhs;
	ast->cond = cond;

	return SUPER(ast);
}

int ast_type_neq(AST* node, ASTType type)
{

	return !(node->type == type);
}

// 函数指针类型
AST* make_type_function_ptr(AST* return_type, AST* declarator, AST* param_list)
{
	CAST(TypenameExpr, ret, return_type);
	CAST(DeclaratorExpr, decl, declarator);

	NEW_STRUCT(Symbol * sym);


}

//
// flags: 是否是 struct, 
AST* make_type(enum Types flags, enum SymbolAttributes attributes, int indirections, const char* name)
{

}

// union/struct
AST* make_aggregate_declare()
{

}