#include "ast.h"
#include "error.h"
#include <assert.h>
#include "types.h"
#include "context.h"
#include <stdio.h>
#include <inttypes.h>
#include "utils.h"
#include <llvm-c/Core.h>

#define NEW_AST(type, name) \
	type* name = (type*) calloc (1, sizeof(type)); \
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


	struct ASTListItem* prev;
	struct ASTListItem* next;
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
	struct ASTList pending_labels;
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
	return item;
}


void ast_init_context(Context* _ctx)
{
	ctx = _ctx;
	NEW_STRUCT(ASTData, data);
	ctx->ast_data = data;
	data->breakable.first = NULL;
	data->breakable.last = NULL;
	data->current_function = NULL;
	data->pending_labels.first = NULL;
	data->pending_labels.last = NULL;
	data->label_id = 0;

	ctx->current = NULL;
	ctx->enums = symtbl_new();
	ctx->functions = symtbl_new();
	ctx->labels = symtbl_new();
	ctx->types = symtbl_new();
	ctx->variables = symtbl_new();
}

uint64_t next_label()
{
	AST_DATA(data);
	return data->label_id++;
}


void init_type_specifier(TypeSpecifier* ts, enum TypeSpecifierFlags flags);

void ast_init(AST* ast, ASTType type)
{
	ast->prev = NULL;
	ast->next = NULL;
	ast->type = type;
	ast->sematic = NULL;
}

void* ast_destroy(AST* rhs)
{
	return NULL;
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
	ast->name = strdup(c);
	ast->val = NULL;
	return SUPER(ast);
}

AST* make_identifier_with_constant_val(const char* c, AST* constant_val)
{
	NEW_AST(IdentifierExpr, ast);
	ast->name = strdup(c);
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
	ast->number_type = type | TP_ROOT_FLAG;

	int is_unsign = type & TP_UNSIGNED;
	int is_hex = str_begin_with(c, "0x");
	int is_oct = str_begin_with(c, "0");
	if (is_hex) is_oct = 0;

#define INT_TYPE_LIST(V)\
V(8, "c") \
V(16, "hd") \
V(32, "d") \
V(64, "lld") 
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
	ast->number_type = TP_STR;
	ast->str = c;

	return SUPER(ast);
}

AST* make_number_float(const char* c, int bits)
{
	NEW_AST(NumberExpr, ast);
	//NEW_AST(OperatorExpr, ast);
	if (bits == 32)
	{
		ast->number_type = TP_FLOAT32;
		ast->f32 = (float)atof(c);
	}
	else {
		ast->number_type = TP_FLOAT64;
		ast->f64 = atof(c);
	}
	ast->number_type |= TP_ROOT_FLAG;
	return SUPER(ast);
}
/*
DeclaratorExpr* ast_apply_specifier_to_declartor(TypeSpecifier* spec, DeclaratorExpr* decl)
{
	decl->attributes = spec->attributes;
	//if (spec->name)
	//{
	//	decl->name = spec->name;
	//}
	if (!decl->first)
	{
		decl->first = decl->last = spec->info;
	}

	if (spec->info)
	{
		type_wrap(decl->last, spec->info);
		decl->last = spec->info;
	}

	return decl;
}
*/

AST* make_unary_expr(enum Operators unary_op, AST* lhs)
{
	NEW_AST(OperatorExpr, ast);
	ast->lhs = lhs;
	ast->op = unary_op;
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
	/*
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
	*/
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
	NEW_AST(LabelStmt, ast);
	ast->target = statements;
	ast->condition = constant;
	ast->type = LABEL_CASE;
	return SUPER(ast);
}

AST* make_label_default(AST* statement)
{
	NEW_AST(LabelStmt, ast);

	ast->target = statement;
	ast->type = LABEL_DEFAULT;
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
	return SUPER(ast);
}

AST* make_jump_cont_or_break(enum JumpType type)
{

	NEW_AST(JumpStmt, ast);
	ast->target = NULL;
	ast->type = type;

	return SUPER(ast);
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
		//if (!data->current_function)
		//{
			//make_error("must return within the function");
		//}
		//else {
			{
				NEW_AST(JumpStmt, ast);
				ast->type = type;
				ast->target = ret;
				ast->ref = data->current_function;
				return SUPER(ast);
			}
			
		//}

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
	NEW_AST(LoopStmt, ast);

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
	ast->exit_label = next_label();
	return SUPER(ast);
}

// Type specifiers Merging
// ======================================
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

int ast_merge_type_qualifier(int a, int b)
{
	if (a & b)
	{
		log_error(NULL, "Duplicated type qualifier");
	}

	return a | b;
}


// Declarators
// ======================================
AST* make_init_direct_declarator(const char* name)
{
	NEW_AST(DeclaratorExpr, ast);
	if (name)
	{
		ast->name = strdup(name);
	}
	ast->init_value = NULL;

	ast->type_spec = NULL;
	ast->type_spec_last = NULL;
	ast->bitfield = NULL;

	return SUPER(ast);
}

void extend_declarator_with_specifier(DeclaratorExpr* decl, TypeSpecifier* spec)
{
	if (decl->type_spec == NULL)
	{
		decl->type_spec = spec;
	}
	else {
		decl->type_spec_last->child = spec;
	}
	while (spec->child)
	{
		spec = spec->child;
	}
	decl->type_spec_last = spec;
}

void extend_declarator_with_specifier_prepend(DeclaratorExpr* decl, TypeSpecifier* spec)
{
	spec->child = decl->type_spec;
	decl->type_spec = spec;
}

AST* make_mark_declarator_paren(AST* target)
{
	CAST(DeclaratorExpr, decl, target);
	if (decl->type_spec)
	{
		decl->type_spec->paren = 1;
	}
	return target;
}

AST* make_extent_direct_declarator(AST* direct, enum Types type, AST* wrapped)
{
	DeclaratorExpr* res;

	if (type == TP_ARRAY)
	{
		CAST(DeclaratorExpr, decl, direct);
		if (wrapped)
		{
			NEW_AST(TypeSpecifier, spec);
			init_type_specifier(spec, TypeSpecifier_Exclusive);
			spec->type = TP_ARRAY;
			spec->array_element_count = wrapped;

			extend_declarator_with_specifier(decl, spec);
		}
		res = decl;
	}
	else // type == TP_FUNC
	{
		TypeSpecifier* prev = NULL;
		TypeSpecifier* first = NULL;
		int i = 0;

		// 把 DeclaratorExpr* 链表串联成 TypeSpecifier* 链表
		FOR_EACH(wrapped, param)
		{
			CAST(DeclaratorExpr, p, param);

			if (p->type_spec == NULL)
			{
				continue;
			}

			p->type_spec->field_name = p->name;
			p->type_spec->super.prev = SUPER(prev);
			p->type_spec->attributes |= p->attributes;

			if (first == NULL)
			{
				first = p->type_spec;
			}
			else {
				prev->super.next = SUPER(p->type_spec);
			}

			prev = p->type_spec;
		}

		NEW_AST(TypeSpecifier, spec);
		init_type_specifier(spec, TypeSpecifier_Exclusive);
		spec->params = first;
		spec->type = TP_FUNC;
		// 如果不是 abstract declarator
		if (direct != NULL)
		{
			CAST(DeclaratorExpr, decl, direct);
			extend_declarator_with_specifier(decl, spec);
			res = decl;
		}
		else {
			NEW_AST(DeclaratorExpr, decl);
			decl->attributes = ATTR_NONE;
			decl->type_spec = decl->type_spec_last = spec;
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
	extend_declarator_with_specifier(decl, ptr->type_spec);
	/*
	ptr->type_spec_last->child = decl->type_spec;
	decl->type_spec = ptr->type_spec;
	if (decl->type_spec_last == NULL)
	{
		decl->type_spec_last = ptr->type_spec_last;
	}*/

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
	decl->bitfield = bitfield;
	return SUPER(decl);
}


AST* make_ptr(int type_qualifier_list, AST* pointing)
{
	NEW_AST(TypeSpecifier, spec);
	init_type_specifier(spec, TypeSpecifier_Exclusive);
	spec->type = TP_PTR;
	spec->attributes = type_qualifier_list;

	if (pointing == NULL)
	{
		// 利用这个函数帮我们初始化
		DeclaratorExpr* ast = (DeclaratorExpr*) make_init_direct_declarator(NULL);
		ast->type_spec = ast->type_spec_last = spec;

		return SUPER(ast);
	}
	else {
		CAST(DeclaratorExpr, decl, pointing);
		extend_declarator_with_specifier(decl, spec);
	}
	return pointing;
}


// Type specifiers
// ======================================
void init_type_specifier(TypeSpecifier* ts, enum TypeSpecifierFlags flags)
{
	ts->array_element_count = NULL;
	ts->attributes = ATTR_NONE;
	ts->child = NULL;
	ts->flags = TypeSpecifier_None;
	ts->name = NULL;
	ts->type = TP_INCOMPLETE;
	ts->flags |= flags;
	ts->params = NULL;
	ts->paren = 0;
}

AST* make_type_specifier(enum Types type)
{
	NEW_AST(TypeSpecifier, ast);
	init_type_specifier(ast, TypeSpecifier_Exclusive);

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
		ast->type = type;
	}
	return SUPER(ast);
}



AST* make_type_specifier_extend(AST* me, AST* other, enum SymbolAttributes storage)
{
	if (me == NULL || other == NULL)
	{
		NEW_AST(TypeSpecifier, ast);
		init_type_specifier(ast, TypeSpecifier_None);
		ast->attributes = storage;
		return SUPER(ast);
	}

	CAST(TypeSpecifier, spec1, me);
	if (other)
	{
		CAST(TypeSpecifier, spec2, other);

#define flag(item__, flag__) (item__->flags & flag__)

#define each_flags(flag1__, flag2__) (flag(spec1, flag1__) && flag(spec2, flag2__))
#define both_flags(flag__) each_flags(flag__, flag__)
#define flag_collision(flag1__, flag2__) (each_flags(flag1__, flag2__) || each_flags(flag2__, flag1__))
#define either_flags(flag__) (flag(spec1, flag__) || flag(spec2, flag__))

		if (both_flags(TypeSpecifier_Exclusive))
		{
			return make_error("Conflicting type specifiers");
		}

#define i32_or_incomplete() \
		(spec1->type == TP_INT32 && spec2->type == TP_INCOMPLETE \
		|| spec1->type == TP_INT32 && spec2->type == TP_INCOMPLETE)

#define one_incomplete() \
		(spec1->type == TP_INCOMPLETE || spec2->type == TP_INCOMPLETE)



		// long long
		if (both_flags(TypeSpecifier_Long))
		{
			if (!i32_or_incomplete())
			{
				log_error(me, "long must be applied to 32 bit integer");
			}

			spec1->flags &= ~TypeSpecifier_Long;
			spec1->flags |= TypeSpecifier_LongLong;
			spec2->type = TP_INCOMPLETE;
		}

		// long int
		else if (flag(spec1, TypeSpecifier_Long) || flag(spec2, TypeSpecifier_Long))
		{
			if (i32_or_incomplete())
			{
				spec1->type = TP_INT32;
			}
			else {
				log_error(me, "long must be applied to 32 bit integer");
			}
			spec2->type = TP_INCOMPLETE;
		}

		// long long int
		else if (flag(spec1, TypeSpecifier_LongLong) || flag(spec2, TypeSpecifier_LongLong))
		{
			if (i32_or_incomplete())
			{
				spec1->type = TP_INT64;
			}
			else {
				log_error(me, "long must be applied to 32 bit integer");
			}
			spec2->type = TP_INCOMPLETE;
		}

		// error: unsigned signed int
		if (both_flags(TypeSpecifier_Unsigned) || both_flags(TypeSpecifier_Signed)
			|| flag_collision(TypeSpecifier_Unsigned, TypeSpecifier_Signed))
		{
			return make_error("Duplicated signed/unsigned specfier");
		}

		if (spec1->type && spec2->type) {
			return make_error("Duplicated type specfier");
		}

		enum Types type = spec1->type | spec2->type;

		if (either_flags(TypeSpecifier_Unsigned))
		{
			if (type & TP_UNSIGNED)
			{
				log_error(me, "Duplicated unsigned specfier");
			}

			type |= TP_UNSIGNED;
		}

		else if (either_flags(TypeSpecifier_Signed))
		{
			if (type & TP_UNSIGNED)
			{
				log_error(me, "Conflicting singned/unsigned specfier");
			}
		}

		spec1->type = type;

		spec1->attributes = ast_merge_storage_specifiers(spec1->attributes, spec2->attributes);
	}
	else {
		spec1->attributes = ast_merge_storage_specifiers(spec1->attributes, storage);
	}
#undef flag
#undef each_flags
#undef both_flags
#undef flag_collision
#undef either_flags
#undef i32_or_incomplete
#undef one_incomplete
	return SUPER(spec1);
}

AST* make_type_specifier_from_id(char* id)
{
	NEW_AST(TypeSpecifier, ast);
	init_type_specifier(ast, TypeSpecifier_Exclusive);
	ast->name = id;
	return SUPER(ast);
}


AST* make_empty()
{
	NEW_AST(EmptyExpr, ast);
	ast->error = NULL;
	return SUPER(ast);
}

AST* make_error(const char* message)
{
	NEW_AST(EmptyExpr, ast);
	ast->error = message;
	assert(0);
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
	DeclaratorExpr* decl = (DeclaratorExpr*)make_init_direct_declarator(NULL);
	NEW_AST(TypeSpecifier, spec);
	init_type_specifier(spec, TypeSpecifier_Exclusive);
	spec->type = TP_ELLIPSIS;

	return SUPER(decl);
}

AST* make_function_call(AST* postfix_expression, AST* params)
{
	NEW_AST(FunctionCallExpr, ast);
	ast->function = postfix_expression;
	ast->params = params;
	ast->function_name = NULL;
	/*

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
	*/

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
	ast->type = SUPER(spec);
	ast->attributes = attribute_specifier;

	return SUPER(ast);
}

// TODO: 这可能是错的, 脑袋有点炸了
AST* make_type_declarator(AST* specifier_qualifier, AST* declarator)
{
	
	CAST(TypeSpecifier, spec, specifier_qualifier);
	CAST(DeclaratorExpr, decl, declarator);


	return make_parameter_declaration(specifier_qualifier, declarator);

	return SUPER(decl);
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
	NEW_AST(EnumDeclareStmt, ast);

	ast->name = identifier;
	ast->ref = NULL;
	
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

		extend_declarator_with_specifier(decl, spec);
	}

	return struct_declarator;
}

AST* make_struct_or_union_define(enum Types type, char* identifier, AST* field_list)
{
	NEW_AST(AggregateDeclareStmt, ast);
	ast->type = type;
	ast->fields = field_list;
	ast->name = identifier;
	ast->ref = NULL;
	return SUPER(ast);
}

AST* ast_merge_specifier_qualifier(AST* me, AST* other, enum SymbolAttributes qualifier)
{
	return make_type_specifier_extend(me, other, qualifier);
}


AST* make_initializer_list(AST* list)
{
	NEW_AST(InitilizerListExpr, ast);
	ast->list = list;
	return SUPER(ast);
}

// 这里解决的是 long a = 0;, 翻译成 int
/*
void normalize_type_specifier(TypeSpecifier* type)
{
	if (type->flags == TypeSpecifier_Long && type->info == NULL)
	{
		type->info = type_fetch_buildtin(TP_INT64);
	}
}*/

AST* make_parameter_declaration(AST* declaration_specifiers, AST* declarator)
{
	CAST(TypeSpecifier, spec, declaration_specifiers);
	DeclaratorExpr* gdecl = NULL;
	if (!declarator)
	{
		gdecl = (DeclaratorExpr*)(make_init_direct_declarator(NULL));
	}
	else {
		CAST(DeclaratorExpr, decl, declarator);
		gdecl = decl;
	}
	
	extend_declarator_with_specifier(gdecl, spec);

	/*
	normalize_type_specifier(spec);

	if (!declarator)
	{
		NEW_AST(DeclaratorExpr, decl);
		decl->attributes = spec->attributes;
		decl->first = spec->info;
		TypeInfo* last = decl->first;
		TypeInfo* last_child = type_get_child(last);
		while (last_child)
		{
			last = last_child;
			last_child = type_get_child(last);
		}
		decl->last = last;
		decl->init_value = NULL;
		decl->name = NULL;
		return SUPER(decl);
	}

	decl = ast_apply_specifier_to_declartor(spec, decl);
	*/

	return SUPER(gdecl);
}

AST* make_define_function(AST* declaration_specifiers, AST* declarator, AST* compound_statement)
{
	NEW_AST(FunctionDefinitionStmt, ast);
	ast->declarator = declarator;
	ast->body = compound_statement;
	ast->specifier = declaration_specifiers;

	return SUPER(ast);
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

AST* make_char(char* c) {
	NEW_AST(NumberExpr, ast);
	ast->number_type = TP_INT8;
	ast->i8 = c[1];

	return SUPER(ast);
}
