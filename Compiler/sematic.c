#include "ast.h"
#include "context.h"
#include "error.h"
#include "symbol.h"
#include "utils.h"


#include <llvm-c/Core.h>

#define SUPER(ptr) &(ptr->super)
#define NOT_IMPLEMENTED return NULL
#define FOR_EACH(ast__, iterator__) \
	for(AST* iterator__ = ast__; iterator__ != NULL; iterator__ = iterator__->next)
#define TRY_CAST(type__, name__, from__) \
	type__* name__ =  NULL; \
	if(from__->type == AST_##type__) \
	{ \
		name__ =  (type__*) (from__); \
	}

struct Context* ctx;

struct SematicData
{
	// LLVMValueRef val;
	TypeInfo* type; // 第二个 pass 推导的类型信息

	LLVMBasicBlockRef basic_block;

	int dummy;
};


struct SematicTempContext
{
	uint64_t temp_id; // llvm 临时变量的 id

	struct SematicTempContext* prev;
};

struct SematicContext
{
	struct SematicTempContext* tmp_bottom;
	struct SematicTempContext* tmp_top;

	LLVMBuilderRef builder;
	LLVMModuleRef module;		// FIX: 同上
} sem_ctx;

char* next_temp_id_str()
{
	uint64_t id = sem_ctx.tmp_top->temp_id++;
	char* buf = (char*)malloc(32);
	snprintf(buf, 32, "%lld", id);
	return buf;
}

static void enter_sematic_temp_context()
{

}

static void leave_sematic_temp_context()
{

}

STRUCT_TYPE(SematicData);


#define FUNC_FORWARD_DECL(t) LLVMValueRef eval_##t(t*);
AST_NODE_LIST(FUNC_FORWARD_DECL)
AST_AUX_NODE_LIST(FUNC_FORWARD_DECL)
#undef FUNC_FORWARD_DECL


void sem_init(AST* ast)
{
	if (ast->sematic == NULL)
	{
		ast->sematic = (SematicData*)malloc(sizeof(SematicData));
		// ast->sematic->val = NULL;
	}
}

LLVMValueRef eval_ast(AST* ast)
{
#define EVAL_CASE(type__) \
	case AST_##type__: return eval_##type__((type__*) ast);
	sem_init(ast);

	switch (ast->type)
	{
		AST_NODE_LIST(EVAL_CASE);
		AST_AUX_NODE_LIST(EVAL_CASE);

	default:
		break;
	}

#undef EVAL_CASE
	return NULL;
}

// returns the value of last eval
LLVMValueRef eval_list(AST* ast)
{
	LLVMValueRef last = NULL;
	while (ast)
	{
		last = eval_ast(ast);
		ast = ast->next;
	}

	return last;
}

// bootstrapping
void do_eval(AST* ast, struct Context* _ctx)
{
	ctx = _ctx;
	eval_list(ast);
}



LLVMValueRef eval_LabelStmt(LabelStmt* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_JumpStmt(JumpStmt* ast)
{
	NOT_IMPLEMENTED;
}


struct DeclaraData
{
	LLVMValueRef value;

};

struct SematicData* sematic_data_new() {
	NEW_STRUCT(SematicData, sem);
	sem->type = NULL;
	return sem;
}

TypeInfo* extract_type(TypeSpecifier* spec)
{
	if (spec == NULL)
	{
		return type_get_error_type();
	}

	enum Types ty = spec->type & TP_CLEAR_SIGNFLAGS;
	enum Types sign_flag = spec->type & TP_GET_SIGNFLAGS;
	char* name = NULL;
	Symbol* sym_in_tbl = NULL;;
	TypeInfo* first = NULL;
	TypeInfo* last = NULL;
	LLVMValueRef array_element_count = NULL;
	TypeInfo* result = type_get_error_type();

	if (ty == TP_INCOMPLETE)
	{
		if (spec->flags & TypeSpecifier_Long)
		{
			spec->type = TP_INT32 | sign_flag;
		}
		if (spec->flags & TypeSpecifier_LongLong)
		{
			spec->type = TP_INT64 | sign_flag;
		}
	}

	spec->flags = TypeSpecifier_None;
	switch (ty)
	{
	case TP_INCOMPLETE:
		log_error(SUPER(spec), "Incomplete type specifier");
		break;

	// Numeric & void types
	case TP_VOID:
	case TP_FLOAT32:
	case TP_FLOAT64:
	case TP_FLOAT128:
	case TP_ELLIPSIS:
		if (sign_flag) {
			log_error(SUPER(spec), "Invalid sign flag on void/float type");
			break;
		}

	// fall through
	case TP_INT8:
	case TP_INT16:
	case TP_INT32:
	case TP_INT64:
	case TP_INT128:
		result = type_fetch_buildtin(spec->type);
		break;

	case TP_STRUCT: // fall through
	case TP_UNION:
		name = str_concat(ty == TP_STRUCT ? "struct " : "union ", spec->name);
		sym_in_tbl = symtbl_find(ctx->types, name);

		if (sym_in_tbl == NULL)
		{
			sym_in_tbl = symbol_create_struct_or_union_incomplete(name, ty);
			symtbl_push(ctx->types, sym_in_tbl);
		}

		free(name);
		result = &sym_in_tbl->type;
		break;

	case TP_ENUM:
		name = str_concat("enum ", spec->name);
		sym_in_tbl = symtbl_find(ctx->types, name);
		if (sym_in_tbl == NULL)
		{
			log_error(SUPER(spec), "'enum %s' is not defined", name);
			break;
		}

		result = &sym_in_tbl->type;
		break;


	case TP_PTR:
		return type_create_ptr(
			spec->attributes,
			extract_type(spec->child)
		);

	case TP_FUNC:
		// 提取函数参数类型
		if (spec->params->type == TP_VOID)
		{
			if(spec->params->super.next)
			{
				log_error(SUPER(spec), "parameter cannot hav void type");
				break;
			}
		}

		AST* params = SUPER(spec->params);
		FOR_EACH(params, ast_ts)
		{
			TRY_CAST(TypeSpecifier, ts, ast_ts);
			TypeInfo* type = extract_type(ts);
			type_append(last, type);
			last = type;
			if (first == NULL)
			{
				first = type;
			}
		}

		result = type_create_func(
			extract_type(spec->child), // 返回值
			spec->name, // 函数名
			first // 函数参数
			);

		break;

	case TP_ARRAY:
		array_element_count = eval_ast(spec->array_element_count);
		if (!LLVMIsConstant(array_element_count))
		{
			log_error(SUPER(spec), "Expected array element count to be constant");
			break;
		}
		;
		result = type_create_array(
			LLVMConstIntGetZExtValue(array_element_count),
			spec->attributes,
			extract_type(spec->child)
		);

		//TODO: Bit field
	case TP_BITFIELD:
		break;
	case TP_LONG_FLAG:
		break;
	default:
		break;
	}
	result->field_name = spec->field_name;
	if (spec->super.sematic == NULL)
	{
		spec->super.sematic = sematic_data_new();
	}
	spec->super.sematic->type = result;
	return result;
}


LLVMValueRef eval_DeclareStmt(DeclareStmt* ast)
{
	FOR_EACH(ast->identifiers, id_ast)
	{
		if (id_ast->type == AST_EmptyExpr)
		{
			eval_EmptyExpr((EmptyExpr*)id_ast);
			continue;
		}

		TRY_CAST(DeclaratorExpr, id, id_ast);
		if (!id)
		{
			log_error(id_ast, "Expected declarator");
			continue;
		}

		

	}
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_EnumDeclareStmt(EnumDeclareStmt* ast)
{
	int64_t enum_val = 0;

	Symbol* enum_type = symbol_create_enum(ast->name ? ast->name : strdup("@anon enum"));
	symtbl_push(ctx->types, enum_type);

	Symbol* first_enum_item = NULL;
	Symbol* last_enum_item = NULL;

	FOR_EACH(ast->enums, enu_ast)
	{
		TRY_CAST(OperatorExpr, op, enu_ast);
		TRY_CAST(IdentifierExpr, id, enu_ast);
		

		if (!op && !id)
		{
			log_internal_error(enu_ast, "Expected constant as enum");
		}

		if (op)
		{
			if (!op->lhs || op->lhs->type != AST_IdentifierExpr)
			{
				log_error(enu_ast, "Lhs must be idenifier");
				continue;
			}
			else if (op->op != OP_ASSIGN)
			{
				log_error(enu_ast, "Only '=' is supported in enum");
				continue;
			}
			LLVMValueRef val = eval_ast(op->rhs);
			if (!LLVMIsConstant(val))
			{
				log_error(enu_ast, "Enum value must be constant!");
				continue;
			}
			enum_val = LLVMConstIntGetSExtValue(val);
			id = (IdentifierExpr*)op->lhs;
		}

		if (id == NULL)
		{
			log_error(enu_ast, "Expected idenfieer");
			continue;
		}

		//TODO: Check for duplicate enums
		Symbol* enum_item = symbol_create_enum_item(
			enum_type, last_enum_item, id->name, 
			LLVMConstInt(LLVMInt64Type(), enum_val, 1));

		symtbl_push(ctx->variables, enum_item);
		variable_append(last_enum_item, enum_item);

		last_enum_item = enum_item;
		if (first_enum_item == NULL)
		{
			first_enum_item = enum_item;
		}

		++enum_val;
	}

	return LLVMConstInt(LLVMInt64Type(), enum_val, 1);
}

// TODO: Add type from symbol table
LLVMValueRef eval_AggregateDeclareStmt(AggregateDeclareStmt* ast)
{
	/*
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
	*/
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_BlockExpr(BlockExpr* ast)
{
	ctx_enter_block_scope(ctx);
	LLVMValueRef last = eval_list(SUPER(ast));
	ctx_leave_block_scope(ctx, 0);

	return last;
}



LLVMValueRef eval_ListExpr(ListExpr* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_FunctionCallExpr(FunctionCallExpr* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_IdentifierExpr(IdentifierExpr* ast)
{
	NOT_IMPLEMENTED;
}

LLVMOpcode eval_binary_opcode_llvm(enum Operators op)
{
	// 全大写(yell) 首字母大写(pascal), c 操作符 op
#define TO_OPCODE(yell__, pascal__, c_op) case yell__: return LLVM##pascal__;

	switch (op)
	{
		
	}
}

static LLVMTypeKind llvm_is_float(LLVMValueRef v)
{
	return LLVMGetTypeKind(LLVMTypeOf(v));//这样好像可以判断类型
}

LLVMValueRef get_numberexpr_llvm_value(NumberExpr* number)
{
	if (number->number_type && TP_INT64)
	{
		return LLVMConstInt(LLVMInt64Type, number->i64, 1);
	}
	else
	{
		return LLVMConstReal(LLVMDoubleType, number->f64);
	}
}

LLVMValueRef get_identifierxpr_llvm_value(IdentifierExpr* identifier)
{
	Symbol* sym = symtbl_find(ctx->variables, identifier->name);
	return sym->var.value;
}

LLVMValueRef eval_OperatorExpr(AST* ast)
{
	LLVMValueRef lhs, rhs;
	if (ast->type & AST_NumberExpr)
	{
		TRY_CAST(NumberExpr, number, ast);
		if (!number)
		{
			log_error(ast, "Expected NumberExpr");
			return NULL;
		}
		return get_numberexpr_llvm_value(number);
	}
	else if (ast->type & AST_IdentifierExpr)
	{
		TRY_CAST(IdentifierExpr, identifier, ast);
		if (!identifier)
		{
			log_error(ast, "Expected IdentifierExpr");
			return NULL;
		}
		return get_identifierxpr_llvm_value(identifier);
	}
	else if (ast->type & AST_OperatorExpr)
	{
		TRY_CAST(OperatorExpr, operator, ast);
		if (!operator)
		{
			log_error(ast, "Expected OperatorExpr");
			return NULL;
		}
		switch (operator->op)
		{
		case OP_ADD:
			lhs = eval_OperatorExpr(operator->lhs);
			rhs = eval_OperatorExpr(operator->rhs);
			return LLVMConstAdd(lhs, rhs);
			break;
		default:
			return NULL;
			break;
		}
	}
	else
	{
		log_error(ast, "Unknown AST Type for function eval_OperatorExpr");
		return NULL;
	}
}

LLVMValueRef eval_InitilizerListExpr(InitilizerListExpr* ast)
{

	
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_EmptyExpr(EmptyExpr* ast)
{
	if (ast->error)
	{
		log_error(SUPER(ast), "Error during parsing: %s", ast->error);
	}
	return NULL;
}

LLVMValueRef eval_LoopStmt(LoopStmt* ast)
{
	NOT_IMPLEMENTED;
}

// TODO: @wushuhui
LLVMValueRef eval_IfStmt(IfStmt* ast) {
	LLVMValueRef condv = eval_ast(ast->condition);
	if (condv == NULL) {
		return NULL;
	}
	// 将condv和0比较, LLVMRealOEQ是否合适, NAN应该是false吧
	// 要Cast吗?
	condv = LLVMBuildFCmp(sem_ctx.builder, LLVMRealOEQ, condv, LLVMConstReal(LLVMDoubleType(), 0.0), "ifcond");
	LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(sem_ctx.builder));
	LLVMBasicBlockRef then_bb = LLVMAppendBasicBlock(func, "then");
	LLVMBasicBlockRef else_bb = LLVMAppendBasicBlock(func, "else");
	LLVMBasicBlockRef merge_bb = LLVMAppendBasicBlock(func, "ifcont");	// 这里应该有两种操作，一种是创建bb并写入，还有一种是创建但不写入

	LLVMBuildCondBr(sem_ctx.builder, condv, then_bb, else_bb);

	LLVMPositionBuilderAtEnd(sem_ctx.builder, then_bb);		// 期望让builder的写指针指向then_bb的末尾，不知道是不是这个
	LLVMValueRef thenv = eval_ast(ast->then);
	if (thenv == NULL) {
		return NULL;	
	}
	LLVMBuildBr(sem_ctx.builder, merge_bb);
	// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
	then_bb = LLVMGetInsertBlock(sem_ctx.builder);

	LLVMPositionBuilderAtEnd(sem_ctx.builder, else_bb);
	LLVMValueRef elsev = eval_ast(ast->otherwise);
	if (elsev == NULL) {
		return NULL;
	}
	LLVMBuildBr(sem_ctx.builder, merge_bb);
	else_bb = LLVMGetInsertBlock(sem_ctx.builder);
	
	LLVMValueRef phi_n = LLVMBuildPhi(sem_ctx.builder, LLVMFloatType(), "iftmp");
	LLVMAddIncoming(phi_n, thenv, then_bb, 0);		// constant 不清楚是不是填这两个
	LLVMAddIncoming(phi_n, elsev, else_bb, 1);
  	return phi_n;
}

// TODO: @wushuhui
LLVMValueRef eval_SwitchCaseStmt(SwitchCaseStmt* ast) {
	NOT_IMPLEMENTED;
}

// TODO: @wushuhui
// main要特殊处理吗?
LLVMValueRef eval_FunctionDefinitionStmt(FunctionDefinitionStmt* ast) {
	TRY_CAST(DeclaratorExpr, decl_ast, ast->declarator);
	if (decl_ast == NULL) {
		return NULL;
	}
	TRY_CAST(TypeSpecifier, type_ast, ast->specifier);
	if (type_ast == NULL) {
		return NULL;
	}

	ctx_enter_function_scope(ctx);

	LLVMValueRef func;

	Symbol* func_sym = symtbl_find(ctx->functions, decl_ast->name);
	if (func_sym == NULL) {
		// 没有声明，那么定义和声明在一起
		LLVMTypeRef func_type = eval_TypeSpecifier(type_ast);
		func = LLVMAddFunction(sem_ctx.module, decl_ast->name, func_type);

		func_sym = symbol_create_func(decl_ast->name, func, extract_type(type_ast), decl_ast->type_spec->params, ast->body);
		symtbl_push(ctx->functions, func_sym);
	} else if (func_sym->func.body != NULL) {
		// 已经有定义了
		log_error(SUPER(ast), "Function Redifinition");
		return NULL;
	} else {
		// 如果符号表中已经有了函数，那么用那个符号
		func = func_sym->value;
		func_sym->func.body = ast->body;
	}

	// TODO 构建实参
	TypeSpecifier* cur = decl_ast->type_spec->params;
	while (cur) {
		
		// symtbl_push(ctx->variables, )
		cur = cur->super.next;
	}

	LLVMBasicBlockRef entry_bb = LLVMAppendBasicBlock(func, "entry");
	// 将参数加入

	LLVMValueRef body = eval_ast(ast->body);
	if (body == NULL) {
		LLVMDeleteFunction(func);
		return NULL;
	}

	LLVMBuildRet(sem_ctx.builder, body);
	// 没有verify吗?

	ctx_leave_function_scope(ctx);
	return func;
}

LLVMValueRef eval_DeclaratorExpr(DeclaratorExpr* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_TypeSpecifier(TypeSpecifier* ast)
{
	NOT_IMPLEMENTED;
}

// Deprecated，为了不编译报错加的
LLVMValueRef eval_NumberExpr(NumberExpr* ast) {
	NOT_IMPLEMENTED;
}

