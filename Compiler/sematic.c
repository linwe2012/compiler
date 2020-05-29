#include "ast.h"
#include "context.h"
#include "error.h"
#include "symbol.h"

#include <llvm-c/Core.h>

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
	int dummy;
};


struct SematicTempContext
{
	uint64_t temp_id; // llvm 临时变量的 id
};

struct SematicContext
{
	struct SematicTempContext* tmp_bottom;
	struct SematicTempContext* tmp_top;

	LLVMBuilderRef builder;
	LLVMContextRef context;		// FIX: 我随便放的，不清楚是不是该放在这里
} sem_ctx;

char* next_temp_id_str()
{
	uint64_t id = sem_ctx.tmp_top->temp_id++;
	char* buf = (char*)malloc(32);
	snprintf(buf, 32, "%ld", id);
	return buf;
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
	while (ast)
	{
		eval_ast(ast);
		ast = ast->next;
	}
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

LLVMValueRef eval_DeclareStmt(DeclareStmt* ast)
{
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
		last_enum_item = enum_item;
		if (first_enum_item == NULL)
		{
			first_enum_item = enum_item;
		}

		++enum_val;
	}

	return LLVMConstInt(LLVMInt64Type(), enum_val, 1);
}

LLVMValueRef eval_AggregateDeclareStmt(AggregateDeclareStmt* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_BlockExpr(BlockExpr* ast)
{
	ctx_enter_block_scope(ctx);
	LLVMValueRef last = eval_list(ast);
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

static int llvm_is_float(LLVMValueRef v)
{
	// LLVMGetTypeKind(LLVMGet)
}

LLVMValueRef eval_OperatorExpr(OperatorExpr* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_InitilizerListExpr(InitilizerListExpr* ast)
{

	
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_EmptyExpr(EmptyExpr* ast)
{
	if (ast->error)
	{
		log_error(ast, "Error during parsing: %s", ast->error);
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
	condv = LLVMConstFCmp(LLVMRealOEQ, condv, LLVMConstReal(LLVMDoubleType(), 0.0));
	LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(sem_ctx.builder));
	LLVMBasicBlockRef then_bb = LLVMAppendBasicBlockInContext(sem_ctx.context, func, "then");
	LLVMBasicBlockRef else_bb = LLVMCreateBasicBlockInContext(sem_ctx.context, "else");
	LLVMBasicBlockRef merge_bb = LLVMCreateBasicBlockInContext(sem_ctx.context, "ifcont");

	LLVMBuildCondBr(sem_ctx.builder, condv, then_bb, else_bb);
	LLVMPositionBuilderAtEnd(sem_ctx.builder, then_bb);	// Builder.SetInsertPoint(ThenBB);

	LLVMValueRef thenv = eval_ast(ast->then);
	if (thenv == NULL) {
		return NULL;	
	}
	LLVMBuildBr(sem_ctx.builder, merge_bb);
	// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
	then_bb = LLVMGetInsertBlock(sem_ctx.builder);
	LLVMAppendExistingBasicBlock(func, merge_bb);		// 这个怎么不存在啊?
	LLVMPositionBuilderAtEnd(sem_ctx.builder, merge_bb);
	
	LLVMValueRef phi_n = LLVMBuildPhi(sem_ctx.builder, LLVMFloatType(), "iftmp");
	// PN->addIncoming(ThenV, ThenBB);
  	// PN->addIncoming(ElseV, ElseBB);
  	return phi_n;
}

// TODO: @wushuhui
LLVMValueRef eval_SwitchCaseStmt(SwitchCaseStmt* ast) {
	NOT_IMPLEMENTED;
}

// TODO: @wushuhui
// 如果没有Declare过,要加SymbolTable
// main要特殊处理吗?
LLVMValueRef eval_FunctionDefinitionStmt(FunctionDefinitionStmt* ast) {
	LLVMValueRef func_type = eval_ast(ast->specifier);
	if (func_type == NULL) {
		return NULL;
	}
}

LLVMValueRef eval_DeclaratorExpr(DeclaratorExpr* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_TypeSpecifier(TypeSpecifier* ast)
{
	NOT_IMPLEMENTED;
}

