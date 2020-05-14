#include "ast.h"
#include "error.h"
#include <assert.h>
#include "types.h"
#include "context.h"
#include <stdio.h>
#include <inttypes.h>
#include "utils.h"

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

struct ASTListItem
{
	AST* ast;
	

	struct ASTList* prev;
	struct ASTList* next;
};


struct ASTList
{
	struct ASTListItem* first;
	struct ASTListItem* last;
};


struct ASTData
{
	// 统计最近的可 break 的 AST
	struct ASTList breakable;
	struct ASTList* pending_labels;
	Symbol* current_function;
	uint64_t label_id;
};

#define AST_DATA(name) struct ASTData* name = (struct ASTData*)ctx->ast_data

Context* ctx;

struct AST* astlist_pop(struct ASTList* target)
{
	struct ASTListItem* item = target->last;
	if (item == NULL)
	{
		target->last = target->first = NULL;
		return NULL;
	}
	else {
		target->last = target->last->prev;
		if (target->last == NULL)
		{
			target->last = target->first = NULL;
		}
	}
	AST* ast = item->ast;
	free(item);
	return ast;
}

struct AST* astlist_back(struct ASTList* target)
{
	struct ASTListItem* item = target->last;
	if (item == NULL)
	{
		return NULL;
	}
	return item->ast;
}

struct ASTListItem* astlist_push(struct ASTList* target, AST* ast)
{
	NEW_STRUCT(ASTListItem, item);
	item->ast = ast;
	item->prev = target->last;
	item->next = NULL;
	target->last->next = item;
	target->last = item;
	if (target->first == NULL)
	{
		target->first = item;
	}
}


void ast_init_context(Context* _ctx)
{
	ctx = _ctx;
	NEW_STRUCT(ASTData, data);
	ctx->ast_data = data;
	data->breakable.first = NULL;
	data->breakable.last = NULL;
	data->label_id = 0;
}

uint64_t next_label()
{
	AST_DATA(data);
	return data->label_id++;
}


void ast_init(AST* ast, ASTType type)
{
	ast->prev = NULL;
	ast->next = NULL;
	ast->type = type;
}

int check_ast(AST* ast)
{
	// return ast->type >= AST_EmptyExpr && ast->type <= AST_FunctionExpr;

	return 1;
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
	ctx_leave_block_scope(ctx, 0);

	return SUPER(ast);
}

void ast_notify_enter_block() {
	ctx_enter_block_scope(ctx);
}

void notify_label(char* name)
{
	// 如果是 label:
	if (name != NULL)
	{
		// label 是否被 goto 定义了, 或者是否重复定义
		Symbol* sym = symtbl_find(ctx->labels, name);
		if (sym != NULL)
		{
			if (sym->label->resolved) {
				log_error(NULL, "duplicated defintion of label: %s", name);
			}
			else {
				sym->label->resolved = 1;
			}
			return;
		}
	}

	Symbol* sym = symbol_create_label(name, next_label(), 1);
	NEW_AST(LabelStmt, ast);
	AST_DATA(data);
	ast->ref = sym;
	if (name)
	{
		symtbl_push(ctx->labels, sym);
	}
	astlist_push(&data->pending_labels, SUPER(ast));
}

AST* make_label(char* name, AST* statement)
{
	AST_DATA(data);
	struct AST* item = astlist_pop(&data->pending_labels);
	CAST(LabelStmt, ast, item);
	ast->target = statement;
	
	return SUPER(ast);
}

AST* make_label_case(AST* constant, AST* statements)
{
	AST_DATA(data);
	struct AST* item = astlist_pop(&data->pending_labels);
	CAST(LabelStmt, ast, item);
	
	ast->target = statements;
	ast->condition = constant;
	return SUPER(ast);
}

AST* make_label_default(AST* statement)
{
	AST_DATA(data);
	struct AST* item = astlist_pop(&data->pending_labels);
	CAST(LabelStmt, ast, item);

	ast->target = statement;

	return SUPER(ast);
}

AST* make_jump_goto(char* name)
{
	AST_DATA(data);
	NEW_AST(JumpStmt, ast);
	ast->ref = symtbl_find(ctx->labels, name);
	ast->type = JUMP_GOTO;
	ast->target = NULL;
	if (ast->ref == NULL)
	{
		ast->ref = symbol_create_label(name, next_label(), 0);
		symtbl_push(ctx->labels, ast->ref);
	}
	return ast;
}

AST* make_jump_cont_or_break(enum JumpType type)
{
	AST_DATA(data);
	struct ASTListItem* item = data->breakable.last;

	if (type == JUMP_CONTINUE)
	{
		while (item)
		{
			if (item->ast->type == AST_LoopStmt)
			{
				break;
			}
			item = item->prev;
		}
	}

	if (item == NULL)
	{
		log_error(NULL, "continue or break must be in a loop or switch case");
	}

	NEW_AST(JumpStmt, ast);
	ast->target = NULL;
	ast->type = type;

	if (item->ast->type == AST_LoopStmt)
	{
		CAST(LoopStmt, loop, item->ast);
		uint64_t id;
		
		if (type == JUMP_CONTINUE)
		{
			id = loop->step_label;
		}
		else {
			id = loop->exit_label;
		}

		ast->ref = symbol_create_label(NULL, id, 1);
	}
	else {
		CAST(SwitchCaseStmt, sc, item->ast);
		ast->ref = symbol_create_label(NULL, sc->exit_label, 1);
	}
	
	return ast;
}

AST* make_jump(enum JumpType type, char* name, AST* ret)
{
	AST_DATA(data);

	switch (type)
	{
	case JUMP_GOTO:
		return make_jump_goto(name);
	case JUMP_CONTINUE: // fall through
	case JUMP_BREAK:
		return make_jump_cont_or_break(type);
	case JUMP_RET:
		if (!data->current_function)
		{
			make_error("must return within the function");
		}
		else {
			NEW_AST(JumpStmt, ast);
			ast->type = type;
			ast->target = ret;
			ast->ref = data->current_function;
			return SUPER(ast);
		}

		break;
	default:
		break;
	}

	return make_error("Internal error");
}

void notify_loop(enum LoopType type)
{
	AST_DATA(data);
	NEW_AST(LoopStmt, ast);
	ast->exit_label = next_label();
	ast->cond_label = next_label();
	ast->loop_type = type;
	if (type == LOOP_FOR)
	{
		ast->step_label = next_label();
	}
	else {
		ast->step_label = ast->cond_label;
	}
	astlist_push(&data->breakable, SUPER(ast));
}

AST* make_loop(AST* condition, AST* before_loop, AST* loop_body, AST* loop_step, enum LoopType loop_type)
{
	AST_DATA(data);
	struct AST* item = astlist_pop(&data->breakable);
	if (item == NULL)
	{
		log_internal_error(item, "corrupted ast data");
	}

	CAST(LoopStmt, ast, item);

	ast->body = loop_body;
	ast->condition = condition;
	ast->enter = before_loop;
	ast->step = loop_step;
	ast->loop_type = loop_type;

	return SUPER(ast);
}



AST* make_ifelse(AST* condition, AST* then, AST* otherwise)
{
	NEW_AST(IfStmt, ast);
	ast->condition = condition;
	ast->then = then;
	ast->otherwise = otherwise;
	return SUPER(ast);
}

AST* make_switch(AST* condition, AST* body)
{
	NEW_AST(SwitchCaseStmt, ast);
	ast->cases = body;
	ast->switch_value = condition;
	return SUPER(ast);
}

int ast_merge_type_qualifier(int a, int b)
{
	if (a & b)
	{
		log_error(NULL, "Duplicated type qualifier");
	}

	return a | b;
}

AST* makr_init_direct_declarator(char* name)
{
	NEW_AST(DeclaratorExpr, ast);
	ast->name = name;
	ast->last = ast->first = NULL;
	ast->init_value = NULL;
	
	return SUPER(ast);
}

AST* make_extent_direct_declarator(AST* direct, enum Types type, AST* wrapped)
{
	CAST(DeclaratorExpr, decl, direct);
	if (type == TP_ARRAY)
	{
		if (wrapped)
		{
			
			CAST(NumberExpr, number, wrapped);
			AST* res = make_binary_expr(OP_CAST, make_type_specifier(TP_INT64 | TP_UNSIGNED), wrapped);
			CAST(NumberExpr, new_num, wrapped);
			AST* last = type_create_array(new_num->i64, ATTR_NONE);
			type_wrap(decl->last, last);
			decl->last = last;
			ast_destroy(res);
		}
	}
	else // type == TP_FUNC
	{
		
		TypeInfo* prev = NULL;
		TypeInfo* first = NULL;
		FOR_EACH(wrapped, param)
		{
			CAST(DeclaratorExpr, p, param);
			p->first->field_name = p->name;
			p->first->prev = prev;
			if (prev)
			{
				prev->next = p->first;
			}
			else {
				first = p->first;
			}
			prev = p->first;
			
		}
		TypeInfo* func = type_create_func(first);

		if (decl->last == NULL)
		{
			decl->last = decl->first = func;
		}
		else {
			type_wrap(decl->last, func);
		}
		
		decl->last = func;
	}

	return SUPER(decl);
}


AST* make_declarator(AST* pointer, AST* direct_declarator)
{
	CAST(DeclaratorExpr, ptr, pointer);
	CAST(DeclaratorExpr, decl, direct_declarator);

	if (decl->last != NULL)
	{
		type_wrap(decl->last, ptr->first);
	}
	else
	{
		decl->last = ptr->first;
	}
	
	decl->last = ptr->last;

	free(ptr);

	return direct_declarator;
}

AST* make_declarator_with_init(AST* declarator, AST* init)
{
	CAST(DeclaratorExpr, decl, declarator);
	decl->init_value = init;

	return SUPER(decl);
}


AST* make_ptr(int type_qualifier_list, AST* pointing)
{
	TypeInfo* type = type_create_ptr(type_qualifier_list);
	
	if (pointing == NULL)
	{
		NEW_AST(DeclaratorExpr, ast);
		ast->first = ast->last = type;
		return ast;
	}
	else {
		CAST(DeclaratorExpr, decl, pointing);
		type_wrap(decl->last, type);
		decl->last = type;
		
	}
	return pointing;
}




int ast_type_neq(AST* node, ASTType type)
{
	return !(node->type == type);
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

/*
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
*/


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