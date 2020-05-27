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
	type__* name__ =  (type__*) (from__)

#define CONSTANT(type__, name__, from__) CAST(type__, name__, from__)

#define FOR_EACH(ast__, iterator__) \
	for(AST* iterator__ = ast__; iterator__ != NULL; iterator__ = iterator__->next)

#define IF_TRY_CAST(type_name__, var__, raw__) \
if (raw__->type == AST_##type_name__) \
{ CAST(type_name__, var__, raw__); 

#define ELSEIF_TRY_CAST(type_name__, var__, raw__) \
}else if (raw__->type == AST_##type_name__) \
{ CAST(type_name__, var__, raw__); 

#define FINALLY_TRY_CAST } else

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
void add_constant_value(NumberExpr* ast, NumberExpr* num, int value, const char* err_msg)
{
	// 由于unsigned占字节一样，因此就不判断是否unsigned了 
	if (num->number_type & TP_INT8)
	{
		ast->i8 = num->i8 + value;
	}
	else if (num->number_type & TP_INT16)
	{
		ast->i16 = num->i16 + value;
	}
	else if (num->number_type & TP_INT32)
	{
		ast->i32 = num->i32 + value;
	}
	else if (num->number_type & TP_INT64)
	{
		ast->i64 = num->i64 + value;
	}
	else if (num->number_type & TP_FLOAT32)
	{
		ast->f32 = num->f32 + value;
	}
	else if (num->number_type & TP_FLOAT64)
	{
		ast->f64 = num->f64 + value;
	}
	else
	{
		err_msg = "This type can not use this operation";
	}
}

DeclaratorExpr* ast_apply_specifier_to_declartor(TypeSpecifier* spec, DeclaratorExpr* decl)
{
	decl->attributes = spec->attributes;
	/*if (spec->name)
	{
		decl->name = spec->name;
	}*/

	if (spec->info)
	{
		type_wrap(decl->last, spec->info);
		decl->last = spec->info;
	}

	return decl;
}

AST* make_unary_expr(enum Operators unary_op, AST* rhs)
{
	NEW_AST(NumberExpr, ast);

	//NEW_AST(OperatorExpr, ast);
	//enum Types type = rhs->sym_type->type.type;
	const char* err_msg = NULL;
	AST* other;
	enum Types type;
	switch (unary_op)
	{
	case OP_INC:
		CAST(NumberExpr, number, rhs);
		ast->number_type = number->number_type;
		add_constant_value(ast, number, 1, err_msg);
		break;
	case OP_DEC:
		CAST(NumberExpr, number, rhs);
		ast->number_type = number->number_type;
		add_constant_value(ast, number, -1, err_msg);
		break;
	case OP_UNARY_STACK_ACCESS:
		break;
	case OP_POSTFIX_INC:
		CAST(NumberExpr, number, rhs);
		ast->number_type = number->number_type;
		add_constant_value(ast, number, 1, err_msg);
		break;
	case OP_POSTFIX_DEC:
		CAST(NumberExpr, number, rhs);
		ast->number_type = number->number_type;
		add_constant_value(ast, number, -1, err_msg);
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
		CAST(NumberExpr, number, rhs);
		ast->number_type = number->number_type;
		ast->f64 = number->f64;
		break;
		/*
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
			//num->super.sym_type = type_fetch_buildtin(TP_INT64 | TP_UNSIGNED);
			num->number_type = TP_INT64 | TP_UNSIGNED;
			//num->ui64 = rhs->sym_type->type.aligned_size;
			other = SUPER(num);
		}

		break;
	}*/
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

	//ast->op = unary_op;
	//ast->rhs = rhs;

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
	DeclaratorExpr* res;

	if (type == TP_ARRAY)
	{
		CAST(DeclaratorExpr, decl, direct);
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
		res = decl;
	}
	else // type == TP_FUNC
	{
		TypeInfo* prev = NULL;
		TypeInfo* first = NULL;

		// 把 AST 链表串联成 TypeInfo* 链表
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

		// 如果不是 abstract declarator
		if (direct != NULL)
		{
			CAST(DeclaratorExpr, decl, direct);


			if (decl->last == NULL)
			{
				decl->last = decl->first = func;
			}
			else {
				type_wrap(decl->last, func);
			}

			decl->last = func;
			res = decl;
		}
		else {
			NEW_AST(DeclaratorExpr, decl);
			decl->attributes = ATTR_NONE;
			decl->first = decl->last = func;
			decl->init_value = NULL;
			decl->name = NULL;
			res = decl;
		}
	}

	return SUPER(res);
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

union ConstantValue constant_from_number_expr(NumberExpr* num)
{
	union ConstantValue* constant = (union ConstantValue*)((&num->number_type) + 1);
	return *constant;
}

AST* make_declarator_bit_field(AST* declarator, AST* bitfield)
{
	CAST(DeclaratorExpr, decl, declarator);
	CONSTANT(NumberExpr, num, bitfield);

	decl->first->bitfield = constant_cast(num->number_type, TP_INT64, constant_from_number_expr(num)).i64;
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

AST* make_type_specifier(enum Types type)
{
	NEW_AST(TypeSpecifier, ast);
	ast->info = NULL;
	ast->flags = TypeSpecifier_None;

	if (type & TP_LONG_FLAG)
	{
		ast->flags = TypeSpecifier_Long;
	}
	else if (type == TP_UNSIGNED)
	{
		ast->flags = TypeSpecifier_Unsigned;
	}
	else if (type == TP_SIGNED)
	{
		ast->flags = TypeSpecifier_Signed;
	}
	else {
		ast->info = type_fetch_buildtin(type);
	}
	ast->attributes = ATTR_NONE;
	
	return ast;
}

// TODO: Add other checks including const/confilicts
int ast_merge_storage_specifiers(int a, int b)
{

	int la = a & ATTR_MASK_STORAGE;
	int lb = b & ATTR_MASK_STORAGE;

	if (la && lb)
	{
		log_error(NULL, "duplicated storage specifiers");
		return a;
	}

	return a | b;
}

AST* make_type_specifier_extend(AST* me, AST* other, enum SymbolAttributes storage)
{
	if (me == NULL || other == NULL)
	{
		NEW_AST(TypeSpecifier, ast);
		ast->attributes = storage;
		ast->flags = TypeSpecifier_None;
		ast->info = NULL;
		ast->name = NULL;
		return SUPER(ast);
	}

	CAST(TypeSpecifier, spec1, me);
	if (other)
	{
		CAST(TypeSpecifier, spec2, other);

#define flag(item__, flag__) (item__->flags == flag__)

#define each_flags(flag1__, flag2__) ((spec1->flags == flag1__) && (spec2->flags == flag2__))
#define both_flags(flag__) each_flags(flag__, flag__)
#define flag_collision(flag1__, flag2__) (each_flags(flag1__, flag2__) || each_flags(flag2__, flag1__))

		if (spec1->info && spec2->info)
		{
			return make_error("Duplicated type specfier");
		}


		// long long
		if (both_flags(TypeSpecifier_Long))
		{
			if (spec1->info || spec2->info)
			{
				log_error(me, "type error");
			}
			spec1->info = type_fetch_buildtin(TP_INT64);
		}
		// long int
		else if (flag(spec1, TypeSpecifier_Long) || flag(spec2, TypeSpecifier_Long))
		{
			TypeInfo* t1 = spec1->info;
			TypeInfo* t2 = spec2->info;

			if (t2)
			{
				t1 = t2;
			}

			if (t1->type != TP_INT32 || t1->type != TP_INT64)
			{
				log_error(me, "long must be applied to 32 bit integer");
			}

			spec1->info = t1;
			spec2->info = NULL;
		}

		// error: unsigned signed int
		if (both_flags(TypeSpecifier_Unsigned) || both_flags(TypeSpecifier_Signed)
			|| flag_collision(TypeSpecifier_Unsigned, TypeSpecifier_Signed))
		{
			return make_error("Duplicated signed/unsigned specfier");
		}


		else if (spec2->info)
		{
			spec1->info = spec2->info;
		}

		spec1->attributes = ast_merge_storage_specifiers(spec1->attributes, spec2->attributes);
	}
	else {
		spec1->attributes = ast_merge_storage_specifiers(spec1->attributes, storage);
	}
#undef flag
#undef each_flags
#undef both_flags
#undef flag_collision

	return SUPER(spec1);
}

AST* make_type_specifier_from_id(char* id)
{
	Symbol* sym = symtbl_find(ctx->types, id);
	if (sym == NULL)
	{
		log_error(NULL, "Undeclared type name");
		return NULL;
	}
	NEW_AST(TypeSpecifier, ast);
	ast->name = id;
	ast->info = &sym->type;
	ast->attributes = ATTR_NONE;
	ast->flags = TypeSpecifier_None;
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


AST* make_list_expr(AST* child)
{
	NEW_AST(ListExpr, list);
	list->first_child = child;
	return SUPER(list);
}

AST* make_paramter_ellipse()
{
	NEW_AST(DeclaratorExpr, decl);
	decl->first = decl->last = type_create_param_ellipse();
	decl->init_value = NULL;

	return SUPER(decl);
}

AST* make_function_call(AST* postfix_expression, AST* params)
{
	NEW_AST(FunctionCallExpr, ast);
	ast->function = postfix_expression;
	ast->params = params;

	if (params != NULL)
	{
		CAST(DeclaratorExpr, decl, params);
		if (decl->first->type == TP_VOID)
		{
			if (decl->super.next != NULL)
			{
				return make_error("void cannot be paramter");
			}
			params = NULL;
		}
	}
	
	int i = 0;
	while (params)
	{
		++i;
		params = params->next;
	}

	ast->n_params = i;
	return SUPER(ast);
}


AST* make_declaration(AST* declaration_specifiers, enum SymbolAttributes attribute_specifier, AST* init_declarator_list)
{
	CAST(TypeSpecifier, spec, declaration_specifiers);
	NEW_AST(DeclareStmt, ast);
	ast->identifiers = init_declarator_list;
	ast->type = spec;

	return SUPER(ast);
}

// TODO: 这可能是错的, 脑袋有点炸了
AST* make_type_declarator(AST* specifier_qualifier, AST* declarator)
{
	CAST(TypeSpecifier, spec, specifier_qualifier);
	CAST(DeclaratorExpr, decl, declarator);
	
	decl = ast_apply_specifier_to_declartor(spec, decl);

	return SUPER(decl);
}

void* ast_destroy(AST* rhs)
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

AST* make_enum_define(char* identifier, AST* enum_list)
{
	char* name = str_concat("enum ", identifier);
	Symbol* sym = symtbl_find(ctx->types, name);
	if (sym != NULL)
	{
		log_error(enum_list, "Enum already declared");
		return make_empty();
	}

	TypeInfo* enum_type = symbol_create_enum(identifier);
	

	TypeInfo* enums_tail = NULL;
	TypeInfo* enums_head = NULL;
	int64_t current_val = 0;

#define APPEND_ENUM(b__) {\
	if (enums_head == NULL) {\
		enums_head = enums_tail = b__;\
		}\
	else {\
		type_append(enums_tail, b__);\
		enums_tail = b__;\
	}\
	}

	FOR_EACH(enum_list, iden)
	{
		IF_TRY_CAST(IdentifierExpr, id, iden) {
			TypeInfo* next = NULL;
			if (id->val == NULL)
			{
				next = symbol_create_enum_item(id->name, current_val);
			}
			else {
				CONSTANT(NumberExpr, num, id->val);
				union ConstantValue val = constant_cast(num->number_type, TP_INT64, constant_from_number_expr(num));
				current_val = val.i64;
				next = symbol_create_enum_item(id->name, current_val);
			}
			if (next != NULL)
			{
				APPEND_ENUM(next);
			}

			++current_val;
			// TODO: Add to symbol tabel
		}
		FINALLY_TRY_CAST{
			log_error(iden, "Expected Indetifier or assigenment expression");
		}
	}
	
	NEW_AST(EnumDeclareStmt, ast);
	ast->ref = symbol_from_type_info(enum_type);
	ast->enums = enum_list;

	return SUPER(ast);
}



AST* make_struct_field_declaration(AST* specifier_qualifier, AST* struct_declarator)
{
	CAST(TypeSpecifier, spec, specifier_qualifier);
	
	FOR_EACH(struct_declarator, st)
	{
		CAST(DeclaratorExpr, decl, st);
		if (decl->init_value)
		{
			log_warning(st, "Struct field init value will be ignored");
		}
		ast_apply_specifier_to_declartor(spec, decl);
		create_struct_field(decl->first, decl->attributes, decl->name);
	}

	return struct_declarator;
}

// TODO: Add type from symbol table
AST* make_struct_or_union_define(enum Types type, char* identifier, AST* field_list)
{
	TypeInfo* first = NULL;
	TypeInfo* last = NULL;

	FOR_EACH(field_list, st)
	{
		CAST(DeclaratorExpr, decl, st);
		TypeInfo* ty = decl->first;
		if (first == NULL)
		{
			first = last = ty;
		}
		else {
			type_append(last, ty);
			last = ty;
		}
	}

	NEW_AST(AggregateDeclareStmt, ast);
	ast->fields = field_list;
	ast->ref = symbol_create_struct_or_union(type_create_struct_or_union(type), first);
	return SUPER(ast);
}

AST* ast_merge_specifier_qualifier(AST* me, AST* other, enum SymbolAttributes qualifier)
{
	return make_type_specifier_extend(me, other, qualifier);
}
/*
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
}*/