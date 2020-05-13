#include "ast.h"
#include "error.h"
#include <assert.h>
#include "types.h"
#include "context.h"
#include <stdio.h>
#include <inttypes.h>

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

AST* make_identifier(const char* c)
{
	NEW_AST(IdentifierExpr, ast);
	ast->name = c;
	ast->val = NULL;
	return SUPER(ast);
}

AST* make_identifier_with_constant_val(const char* c, AST* constant_val)
{
	NEW_AST(IdentifierExpr, ast);
	ast->name = c;
	ast->val = constant_val;
	return SUPER(ast);
}

int str_begin_with(const char* a, const char* prefix)
{
	while (*a && *prefix)
	{
		if (*a != *prefix) break;
		++a;
		++prefix;
	}

	return *a == *prefix;
}



AST* make_number_int(char* c, enum Types type)
{
	NEW_AST(NumberExpr, ast);
	ast->number_type = type;

	int is_unsign = type & TP_UNSIGNED;
	int is_hex = str_begin_with(c, "0x");
	int is_oct = str_begin_with(c, "0");
	if (is_hex) is_oct = 0;

#define INT_TYPE_LIST(V)\
V(8, "hh") \
V(16, "h") \
V(32, "l") \
V(64, "ll") 
	switch (type & TP_CLEAR_SIGNFLAGS)
	{
#define TYPE_CASE(bits, scn) \
	case TP_INT##bits: {\
		if (is_unsign)\
		{\
			if (is_hex) sscanf(c, "%" scn "ux", &(ast->ui##bits));\
			else if (is_oct) sscanf(c, "%" scn "uo", &(ast->ui##bits));\
			else sscanf(c, "%" scn "u", &(ast->ui##bits));\
		}\
		else {\
			if (is_hex) sscanf(c, "%" scn "x", &(ast->ui##bits));\
			else if (is_oct) sscanf(c, "%" scn "o", &(ast->ui##bits));\
			else sscanf(c, "%" scn "", &(ast->ui##bits));\
		}\
	}
	INT_TYPE_LIST(TYPE_CASE)
	
#undef TYPE_CASE
#undef INT_TYPE_LIST
	default:
		break;
	}

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

/*

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
}*/

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


AST* make_block(AST* first_child)
{
	NEW_AST(BlockExpr, ast);

	ast->first_child = first_child;

	return SUPER(ast);
}

AST* make_label(char* name, AST* statement)
{
	NEW_AST(LabelStmt, ast);
	ast->label = name;
	ast->target = statement;
}

AST* make_label_case(AST* constant, AST* statements)
{
	switch (10)
	{
		char c = 10;
		++constant;
	case 2:
		++constant;
		++statements;
		++c;
	default:
		break;
	}
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





int ast_type_neq(AST* node, ASTType type)
{
	return !(node->type == type);
}


//
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

void* ast_destroy(AST* rhs)
{

}

Value request_label(Context* ctx)
{

}


Value handle_AST(Context* ctx, SwitchCaseStmt* ast)
{

}

struct ListItem
{
	struct ListItem* next;
	Value val;
};
struct List
{
	struct ListItem* head;
	struct ListItem* tail;
};

void list_init(struct List* list)
{
	list->head = list->tail = NULL;
}

void list_append(struct List* list, Value val)
{
	if (list->head == NULL)
	{
		list->head = list->tail = malloc(sizeof(Value));
		list->head->next = NULL;
		list->head->val = val;
	}

	else {
		list->tail->next = malloc(sizeof(Value));
		list->tail = list->tail->next;
		list->tail->next = NULL;
		list->tail->val = val;
	}
}

void list_destroy(struct List* list)
{
	struct ListItem* item = list->head;
	struct ListItem* next;
	while (item)
	{
		next = item->next;
		free(item);
		item = next;
	}
}

Value handle_SwitchCaseStmt(Context*ctx, SwitchCaseStmt* ast)
{
	Value val = handle_AST(ctx, ast->cases);
	Value exit = request_label(ctx);
	struct List list;
	
	FOR_EACH(ast->switch_value, stmt)
	{
		while (stmt && stmt->type != AST_LabelStmt)
		{
			stmt = stmt->next;
		}
		LabelStmt* label = (LabelStmt*)stmt;
		if (!label->condition)
		{
			continue;
		}
		
		Value cond = handle_AST(ctx, label->condition);
		Value case_label = request_label(ctx);

		list_append(&list, case_label);

		ctx->gen->cmp_(val, cond);
		ctx->gen->je_(exit);
	}

	FOR_EACH(ast->switch_value, stmt)
	{
		while (stmt && stmt->type != AST_LabelStmt)
		{
			stmt = stmt->next;
		}
		LabelStmt* label = (LabelStmt*)stmt;
		if (!label->condition)
		{
			continue;
		}

		Value cond = handle_AST(ctx, label->condition);
		
	}

	list_destroy(&list);
}