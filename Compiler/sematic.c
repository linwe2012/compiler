#include "ast.h"
#include "context.h"
#include "error.h"
#include "symbol.h"
#include "utils.h"


#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ErrorHandling.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>
#include <llvm-c/Transforms/Vectorize.h>
#include <llvm-c/Transforms/IPO.h>
#include <llvm-c/Transforms/Scalar.h>

#define SUPER(ptr) &(ptr->super)
#define NOT_IMPLEMENTED return NULL
#define FOR_EACH(ast__, iterator__) \
	for(AST* iterator__ = ast__; iterator__ != NULL; iterator__ = iterator__->next)
#define TRY_CAST(type__, name__, from__) \
	type__* name__ =  NULL; \
	if(from__ != NULL && from__->type == AST_##type__) \
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


typedef struct SematicTempContext {
	uint64_t temp_id; // llvm 临时变量的 id

	struct SematicTempContext* prev;
	struct SematicTempContext* next;
} SematicTempContext;

typedef struct BBListNode {
	LLVMBasicBlockRef block;
	struct BBListNode* next;
	struct BBListNode* prev;
} BBListNode;

struct SematicContext
{
	SematicTempContext* tmp_top;

	BBListNode* breakable_last;		// break栈
	BBListNode* continue_last;		// continue栈
	BBListNode* after_bb;			// 不能简单使用AppendBasicBlock

	Symbol* cur_func_sym;		// 这个是从symtbl里面查出来的

	LLVMBuilderRef builder;
	LLVMModuleRef module;
} sem_ctx;

// TypeInfo.value = TypeLLVMValue
struct TypeSematic
{
	LLVMTypeRef llvm_type;
};

static void append_bb(BBListNode** top, LLVMBasicBlockRef bb) {
	BBListNode* breakable = (BBListNode*)calloc(1, sizeof(BBListNode));
	breakable->block = bb;
	if (*top == NULL) {
		*top = breakable;
	}
	else {
		breakable->prev = *top;
		(*top)->next = breakable;
		*top = breakable;
	}
}

static void pop_bb(BBListNode** top) {
	BBListNode* new_last = (*top)->prev;
	free(*top);
	*top = new_last;
	if (new_last != NULL) {
		new_last->next = NULL;
	}
}

static void free_bb_stack(BBListNode** top) {
	BBListNode* tmp;
	while (*top != NULL) {
		tmp = *top;
		*top = (*top)->prev;
		free(tmp);
	}
}

static char* next_temp_id_str() {
	uint64_t id = sem_ctx.tmp_top->temp_id++;
	char* buf = (char*)malloc(32);
	snprintf(buf, 32, "%lld", id);
	return buf;
}

static void enter_sematic_temp_context() {
	SematicTempContext* new_seman = (SematicTempContext*)calloc(1, sizeof(SematicTempContext));
	if (sem_ctx.tmp_top == NULL) {
		sem_ctx.tmp_top = new_seman;
	}
	else {
		new_seman->prev = sem_ctx.tmp_top;
		sem_ctx.tmp_top->next = new_seman;
		sem_ctx.tmp_top = new_seman;
	}
}

static void leave_sematic_temp_context() {
	BBListNode* new_last = sem_ctx.tmp_top->prev;
	free(sem_ctx.tmp_top);
	sem_ctx.tmp_top = new_last;
	if (new_last != NULL) {
		new_last->next = NULL;
	}
}

static LLVMBasicBlockRef alloc_bb(char* name) {
	if (sem_ctx.after_bb) {
		return LLVMInsertBasicBlock(sem_ctx.after_bb->block, name);
	}
	else {
		return LLVMAppendBasicBlock(LLVMGetBasicBlockParent(LLVMGetInsertBlock(sem_ctx.builder)), name);
	}
}

// 支持putchar, 方便测试的时候用
static void build_putchar() {
	// int putchar(int c);
	LLVMTypeRef* argv = malloc(sizeof(LLVMTypeRef));
	argv[0] = LLVMInt32Type();
	LLVMTypeRef func_tp = LLVMFunctionType(LLVMInt32Type(), argv, 1, 0);

	LLVMValueRef func = LLVMAddFunction(sem_ctx.module, "putchar", func_tp);
	Symbol* func_sym = symbol_create_func("putchar", func, LLVMInt32Type(), argv, NULL, 0, 1);
	symtbl_push(ctx->functions, func_sym);
}

static LLVMValueRef llvm_convert_type(LLVMTypeRef dest_type, LLVMValueRef val);

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

void sematic_init_context(Context* ctx)
{
	// TODO
	//LLVM
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

static int llvm_is_b(LLVMValueRef inst) {
	return LLVMIsAReturnInst(inst) || LLVMIsABranchInst(inst) || LLVMIsAIndirectBrInst(inst);
}

static int llvm_is_float(LLVMValueRef v) {
	return LLVMTypeOf(v) == LLVMFloatType()
		|| LLVMTypeOf(v) == LLVMDoubleType();
}

static int llvm_is_int(LLVMValueRef v) {
	return LLVMTypeOf(v) == LLVMInt64Type()
		|| LLVMTypeOf(v) == LLVMInt32Type()
		|| LLVMTypeOf(v) == LLVMInt16Type()
		|| LLVMTypeOf(v) == LLVMInt8Type()
		|| LLVMTypeOf(v) == LLVMInt1Type()
		|| LLVMTypeOf(v) == LLVMInt128Type();
}

static int llvm_is_bit(LLVMValueRef v) {
	return LLVMTypeOf(v) == LLVMInt1Type();
}

static int type_is_float(LLVMTypeRef type) {
	return type == LLVMFloatType()
		|| type == LLVMDoubleType();
}

static int type_is_int(LLVMTypeRef type) {
	return type == LLVMInt1Type() ||
		type == LLVMInt8Type() ||
		type == LLVMInt16Type() ||
		type == LLVMInt32Type() ||
		type == LLVMInt64Type();
}

static LLVMValueRef get_OperatorExpr_LeftValue(AST* ast);
static LLVMTypeRef extract_llvm_type(TypeInfo* info);


/***********************************************************************************/


// returns the value of last eval
LLVMValueRef eval_list(AST* ast)
{
	LLVMValueRef last = NULL;
	while (ast) {
		last = eval_ast(ast);
		if (llvm_is_b(last)) {
			// 如果有跳转，直接中断同一层后续ast的编译
			// 这段代码在加入jump后需要调整，因为没有jump情况下跳转一定不会跳到同bb的后续代码中
			// 有jump就不好说了
			break;
		}
		ast = ast->next;
	}

	return last;
}

// bootstrapping
void do_eval(AST* ast, struct Context* _ctx, char* module_name, const char* output_file)
{
	sem_ctx.module = LLVMModuleCreateWithName(module_name);
	sem_ctx.builder = LLVMCreateBuilder();
	sem_ctx.cur_func_sym = sem_ctx.breakable_last = sem_ctx.continue_last = NULL;
	sem_ctx.tmp_top = NULL;

	LLVMPassManagerRef passes = LLVMCreatePassManager();
	LLVMAddMergedLoadStoreMotionPass(passes);
	LLVMAddVerifierPass(passes);

	ctx = _ctx;
	build_putchar();		// 内建putchar
	eval_list(ast);

	LLVMRunPassManager(passes, sem_ctx.module);
	LLVMDisposePassManager(passes);

	char** msg = NULL;
	LLVMBool res = LLVMPrintModuleToFile(sem_ctx.module, output_file, msg);
	if (res != 0 && msg != NULL) {
		printf("LLVM error: %s\n", msg);
		LLVMDisposeMessage(*msg);
	}
	LLVMDisposeBuilder(sem_ctx.builder);
	LLVMDisposeModule(sem_ctx.module);
}



LLVMValueRef eval_LabelStmt(LabelStmt* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_JumpStmt(JumpStmt* ast) {
	switch (ast->type) {
	case JUMP_BREAK:
		if (sem_ctx.breakable_last == NULL) {
			log_error(SUPER(ast), "No loop to break");
		}
		else {
			LLVMBuildBr(sem_ctx.builder, sem_ctx.breakable_last->block);
		}
		break;
	case JUMP_CONTINUE:
		if (sem_ctx.continue_last == NULL) {
			log_error(SUPER(ast), "No loop to continue");
		}
		else {
			LLVMBuildBr(sem_ctx.builder, sem_ctx.continue_last->block);
		}
		break;
	case JUMP_RET:
		if (ast->target) {
			LLVMValueRef ret = eval_ast(ast->target);
			// 如果类型不同，强制trunc，这段代码依然有问题
			if (LLVMTypeOf(ret) != sem_ctx.cur_func_sym->func.ret_type) {
				ret = LLVMBuildTrunc(sem_ctx.builder, ret, sem_ctx.cur_func_sym->func.ret_type, next_temp_id_str());
			}
			LLVMBuildRet(sem_ctx.builder, ret);
		}
		else {
			LLVMBuildRetVoid(sem_ctx.builder);
		}
		break;
	case JUMP_GOTO:
	default:
		break;
	}
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

char* get_struct_or_union_name(enum Types type, const char* name)
{
	return (
		str_concat(type == TP_STRUCT ? "struct " : "union ", name)
		);
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
		if (spec->child)
		{
			log_error(SUPER(spec), "Incompatible type specifier");
			break;
		}
		result = type_fetch_buildtin(spec->type, spec->field_name);
		break;

	case TP_STRUCT: // fall through
	case TP_UNION:
		if (spec->child)
		{
			log_error(SUPER(spec), "Incompatible type specifier");
			break;
		}

		if (spec->struct_or_union) {
			TypeInfo* first = NULL;
			TypeInfo* last = NULL;
			AST* field_list = spec->struct_or_union;
			Symbol* aggregate = NULL;
			int found_in_symtbl = 0;
			char* type_name = get_struct_or_union_name(spec->type, spec->name);
			if (spec->name != NULL) {
				aggregate = symtbl_find(ctx->types, type_name);
				if (aggregate) {
					found_in_symtbl = 1;
					if (!aggregate->type.incomplete) {
						result = &aggregate->type;
						break;
					}
				}
			}

			if (aggregate == NULL) {
				aggregate = symbol_create_struct_or_union_incomplete(NULL, spec->type);
			}

			if (!found_in_symtbl && spec->name != NULL)
			{
				aggregate->name = type_name;

				NEW_STRUCT(TypeSematic, sem);
				sem->llvm_type = LLVMStructCreateNamed(LLVMGetGlobalContext(), aggregate->name);
				aggregate->type.value = sem;

				symtbl_push(ctx->types, aggregate);
			}
			else {
				free(type_name);
			}

			if (field_list == NULL) {
				log_error(SUPER(spec), "Expected at least on field for struct");
			}

			int field_cnt = 0;

			FOR_EACH(field_list, st) {
				field_cnt++;
				TRY_CAST(DeclaratorExpr, decl, st);
				// TODO: 可能不是 block scope
				ctx_enter_block_scope(ctx);
				decl->type_spec->field_name = decl->name;

				TypeInfo* ty = extract_type(decl->type_spec);
				ctx_leave_block_scope(ctx, 0);

				if (first == NULL) {
					first = last = ty;
				}
				else {
					type_append(last, ty);
					last = ty;
				}
			}


			LLVMValueRef* llvm_fields = NULL;
			if (spec->type == TP_STRUCT) {
				llvm_fields = malloc(sizeof(LLVMValueRef) * field_cnt);
				TypeInfo* cur = first;
				for (int i = 0; i < field_cnt; ++i) {
					llvm_fields[i] = extract_llvm_type(cur);
					cur = cur->next;
				}
			}
			else if (spec->type == TP_UNION) {
				// TODO union的话只有最大的那个field
				field_cnt = 1;
			}


			if (first != NULL)
			{
				symbol_create_struct_or_union(&aggregate->type, first);

				struct TypeSematic* sem = aggregate->type.value;
				// sem->llvm_type = LLVMArrayType(LLVMInt8Type(), aggregate->type.aligned_size);

				LLVMStructSetBody(sem->llvm_type, llvm_fields, field_cnt, 0);

				result = &aggregate->type;
			}
			break;
		}
		else {
			name = str_concat(ty == TP_STRUCT ? "struct " : "union ", spec->name);
			sym_in_tbl = symtbl_find(ctx->types, name);

			if (sym_in_tbl == NULL)
			{
				sym_in_tbl = symbol_create_struct_or_union_incomplete(name, ty);
				symtbl_push(ctx->types, sym_in_tbl);
			}
			else {
				free(name);
			}

			result = &sym_in_tbl->type;
			break;
		}



	case TP_ENUM:
		name = str_concat("enum ", spec->name);
		sym_in_tbl = symtbl_find(ctx->types, name);
		if (sym_in_tbl == NULL)
		{
			log_error(SUPER(spec), "'%s' is not defined", name);
			break;
		}

		result = &sym_in_tbl->type;
		break;


	case TP_PTR:
		return type_create_ptr(
			spec->attributes,
			extract_type(spec->child),
			spec->field_name
		);

	case TP_FUNC:
		// 提取函数参数类型
		if (spec->params->type == TP_VOID)
		{
			if (spec->params->super.next)
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


LLVMTypeRef extract_llvm_type(TypeInfo* info) {
	TypeInfo* ptr = NULL;

	switch (info->type & TP_CLEAR_SIGNFLAGS) {
	case TP_INT8:
		return LLVMInt8Type();
	case TP_INT16:
		return LLVMInt16Type();
	case TP_INT32:
		return LLVMInt32Type();
	case TP_INT64:
		return LLVMInt64Type();
	case TP_INT128:
		return LLVMInt128Type();
	case TP_VOID:
		return LLVMVoidType();
	case TP_FLOAT32:
		return LLVMFloatType();
	case TP_FLOAT64:
		return LLVMDoubleType();
	case TP_PTR:
		// void* 被当作 i8*, 因为 llvm 不支持 void *
		if (info->ptr.pointing->type == TP_VOID)
		{
			return LLVMPointerType(LLVMInt8Type(), 0);
		}

		ptr = extract_llvm_type(info->ptr.pointing);
		return LLVMPointerType(ptr, 0);

	case TP_ARRAY:
		return LLVMArrayType(extract_llvm_type(info->arr.array_type), info->arr.array_count);
	case TP_ENUM:
		return LLVMInt64Type();
	case TP_UNION:
	case TP_STRUCT:
		// return LLVMArrayType(LLVMInt8Type(), info->aligned_size);
		return ((struct TypeSematic*)info->value)->llvm_type;

	default:
		break;
	}
	return NULL;
}

// 返回函数符号
static Symbol* eval_FuncDeclareStmt(AST* ast, TypeSpecifier* ret_typesp, const char* name, TypeSpecifier* paramsp) {
	Symbol* func_sym = symtbl_find(ctx->functions, name);
	if (func_sym != NULL) {
		log_error(ast, "Function redeclaration");
		return NULL;
	}
	LLVMTypeRef ret_type = extract_llvm_type(extract_type(ret_typesp));
	LLVMTypeRef* argv = NULL;
	int argc = 0;

	int is_var_arg = 0;
	// 构建形参
	// 如果第一个参数是void，代表无参数, TODO：检查void后面还跟了参数的情况(低优先级)
	if (paramsp != NULL && paramsp->type != TP_VOID) {
		TypeSpecifier* tmp = paramsp;
		TypeSpecifier* prev = NULL;

		for (argc = 0; tmp != NULL && tmp->type != TP_ELLIPSIS; ++argc, tmp = tmp->super.next)
			prev = tmp;

		// 如果是变参函数
		if (tmp)
		{
			// 在  '...' 之后声明参数是无效的, 但是这个 Yacc 已经帮我们检查了

			is_var_arg = 1;
		}
		argv = (LLVMTypeRef*)malloc(argc * sizeof(LLVMTypeRef));
		tmp = paramsp;
		for (int i = 0; i < argc; ++i) {
			argv[i] = extract_llvm_type(extract_type(tmp));
			tmp = tmp->super.next;
		}
	}


	LLVMTypeRef func_type = LLVMFunctionType(
		ret_type,	// 返回类型
		argv,		// 形参数组
		argc,		// 形参数量
		is_var_arg			// TODO 支持...
	);
	LLVMValueRef func = LLVMAddFunction(sem_ctx.module, name, func_type);
	enum SymbolAttributes attributes = ATTR_NONE;
	TRY_CAST(FunctionDefinitionStmt, fn_stmt, ast);
	TRY_CAST(DeclareStmt, decl_stmt, ast);

	if (fn_stmt)
	{
		TRY_CAST(TypeSpecifier, spec, fn_stmt->specifier);
		attributes = spec->attributes;
	}
	else if (decl_stmt)
	{
		attributes = decl_stmt->attributes;
	}

	if (attributes != ATTR_NONE)
	{
		if (attributes & ATTR_STDCALL)
		{
			LLVMSetFunctionCallConv(func, LLVMX86StdcallCallConv);
		}
		else if (attributes & ATTR_FASTCALL)
		{
			LLVMSetFunctionCallConv(func, LLVMX86FastcallCallConv);
		}

	}

	func_sym = symbol_create_func(name, func, ret_type, argv, NULL, is_var_arg, argc);
	symtbl_push(ctx->functions, func_sym);
	return func_sym;
}

static LLVMValueRef llvm_get_default(LLVMTypeRef type) {
	if (llvm_is_int(type)) {
		return LLVMConstInt(type, 0, 1);
	}
	else if (llvm_is_float(type)) {
		return LLVMConstReal(type, 0);
	}
	else {
		// TODO 更多的默认类型
		return NULL;
	}
}

LLVMValueRef eval_DeclareStmt(DeclareStmt* ast)
{
	TRY_CAST(TypeSpecifier, spec, ast->type);
	LLVMValueRef last_value = NULL;

	if (spec && (spec->type == TP_STRUCT || spec->type == TP_UNION))
	{
		if (ast->identifiers == NULL)
		{
			extract_type(spec);
		}
	}

	FOR_EACH(ast->identifiers, id_ast) {
		if (id_ast->type == AST_EmptyExpr) {
			eval_EmptyExpr((EmptyExpr*)id_ast);
			continue;
		}

		TRY_CAST(DeclaratorExpr, id, id_ast);

		if (!id) {
			log_error(id_ast, "Expected declarator");
			continue;
		}
		else if (!id->name) {
			log_error(id_ast, "Expected name for declarator");
			continue;
		}

		if (id->type_spec != NULL && id->type_spec->type == TP_FUNC) {
			// 应该是这样区分函数和变量的吧
			eval_FuncDeclareStmt(SUPER(ast), spec, id->name, id->type_spec->params);
			last_value = NULL;
		}
		else {
			if (symtbl_find_in_current_scope(ctx->variables, id->name)) {
				log_error(id_ast, "Redeclaration of variable %s", id->name);
				continue;
			}

			extend_declarator_with_specifier(id, spec);
			TRY_CAST(TypeSpecifier, id_spec, (SUPER(id->type_spec)));
			if (id_spec == NULL)
			{
				log_error(id_ast, "Identifier has no type specifier");
				continue;
			}

			TypeInfo* type_info = extract_type(id_spec);
			// TODO 更加复杂的构建
			LLVMTypeRef decl_type = extract_llvm_type(type_info);

			LLVMValueRef ptr = NULL;
			LLVMValueRef value = NULL;
			if (ctx->variables->stack_top->prev == NULL) {
				ptr = LLVMAddGlobal(sem_ctx.module, decl_type, id->name);
				if (id_spec->attributes != ATTR_EXTERN) {
					if (id->init_value) {
						value = eval_ast(id->init_value);
						if (LLVMTypeOf(value) != decl_type) {
							value = llvm_convert_type(decl_type, value);
						}
						last_value = value;
					}
					else {
						// 没有初始化的也要做初始化
						value = llvm_get_default(decl_type);
					}
					LLVMSetInitializer(ptr, value);
				}
			}
			else {
				ptr = LLVMBuildAlloca(sem_ctx.builder, decl_type, id->name);

				// TODO: 检查合并 attributes 的时候问题
				id->attributes |= ast->attributes;
				if (id->init_value)
				{
					value = eval_ast(id->init_value);
					if (LLVMTypeOf(value) != decl_type) {
						value = llvm_convert_type(decl_type, value);
					}
					last_value = value;
					LLVMBuildStore(sem_ctx.builder, value, ptr);
				}
			}
			Symbol* sym = symbol_create_variable(id->name, id->attributes, symbol_from_type_info(type_info), ptr, 0);


			symtbl_push(ctx->variables, sym);
		}
	}
	return last_value;
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

// TODO: 这个不需要了, AggregateDeclareStmt 已经没了
LLVMValueRef eval_AggregateDeclareStmt(AggregateDeclareStmt* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_BlockExpr(BlockExpr* ast)
{
	ctx_enter_block_scope(ctx);
	LLVMValueRef last = eval_list(ast->first_child);
	ctx_leave_block_scope(ctx, 0);

	return last;
}

// FunctionDefinition和ForLoop的scope是手动创建的
LLVMValueRef eval_BlockExprNoScope(AST* ast) {
	TRY_CAST(BlockExpr, body_ast, ast);
	if (body_ast == NULL) {
		return NULL;
	}
	return eval_list(body_ast->first_child);
}


// 对于 Initializer List 不能调用这个
LLVMValueRef eval_ListExpr(ListExpr* ast)
{
	return eval_list(ast->first_child);
}

LLVMValueRef eval_FunctionCallExpr(FunctionCallExpr* ast) {
	TRY_CAST(IdentifierExpr, func_ast, ast->function);
	if (func_ast == NULL) {
		log_error(SUPER(ast), "Empty function identifier ast");
		return NULL;
	}
	Symbol* func_sym = symtbl_find(ctx->functions, func_ast->name);
	if (func_sym == NULL) {
		log_error(SUPER(ast), "Called function not declared");
		return NULL;
	}
	unsigned int argc = 0;
	AST* arg = ast->params;
	for (; arg != NULL; arg = arg->next, ++argc);
	LLVMValueRef* argv = calloc(argc, sizeof(LLVMValueRef));

	arg = ast->params;
	int i = 0;
	for (; i < func_sym->func.argc; ++i)
	{
		argv[i] = llvm_convert_type(func_sym->func.params[i], eval_ast(arg));
		if (argv[i] == NULL) {
			log_error(SUPER(ast), "Pass null value to function call");
			return NULL;
		}
		arg = arg->next;
	}

	if (func_sym->func.is_variadic)
	{
		for (; arg != NULL; ++i) {
			argv[i] = eval_ast(arg);
			arg = arg->next;
		}
	}

	return LLVMBuildCall(sem_ctx.builder,
		func_sym->func.value, argv, argc,
		func_sym->func.ret_type == LLVMVoidType() ? "" : next_temp_id_str());
}


LLVMOpcode eval_binary_opcode_llvm(enum Operators op)
{
	// 全大写(yell) 首字母大写(pascal), c 操作符 op
#define TO_OPCODE(yell__, pascal__, c_op) case yell__: return LLVM##pascal__;

	switch (op)
	{

	}
}



LLVMValueRef eval_IdentifierExpr(IdentifierExpr* ast)
{
	Symbol* sym = symtbl_find(ctx->variables, ast->name);
	return LLVMBuildLoad(sem_ctx.builder, sym->var.value, next_temp_id_str());
}

// TODO: support more types
LLVMValueRef eval_NumberExpr(NumberExpr* ast) {
	enum Types type = (ast->number_type & 0xFu);
	switch (type) {
	case TP_INT8:
		return LLVMConstInt(LLVMInt8Type(), ast->i8, 1);
	case TP_INT32:
		return LLVMConstInt(LLVMInt32Type(), ast->i32, 1);
	case TP_INT64:
		return LLVMConstInt(LLVMInt64Type(), ast->i64, 1);
	case TP_STR:
		return LLVMBuildGlobalStringPtr(sem_ctx.builder, ast->str, next_temp_id_str());
	case TP_FLOAT32:
		return LLVMConstReal(LLVMFloatType(), ast->f32);
	case TP_FLOAT64:
		return LLVMConstReal(LLVMDoubleType(), ast->f64);
	default:
		log_error(SUPER(ast), "type %d currently not supported", type);
	}
	return NULL;
}

LLVMValueRef get_identifierxpr_llvm_value(IdentifierExpr* expr) {
	Symbol* sym = symtbl_find(ctx->variables, expr->name);
	if (sym == NULL) {
		return NULL;
	}
	return LLVMBuildLoad(sem_ctx.builder, sym->value, expr->name);
}

void save_identifierexpr_llvm_value(IdentifierExpr* expr, LLVMValueRef val)
{
	Symbol* sym = symtbl_find(ctx->variables, expr->name);
	if (sym == NULL) {
		log_error(SUPER(expr), "No such identiifer: %s");
	}
	LLVMTypeKind kind = LLVMGetTypeKind(LLVMTypeOf(val));
	LLVMBuildStore(sem_ctx.builder, val, sym->value);
}

LLVMValueRef llvm_convert_type(LLVMTypeRef dest_type, LLVMValueRef val)
{
	LLVMTypeRef type = LLVMTypeOf(val);
	LLVMTypeKind kind = LLVMGetTypeKind(type);
	LLVMTypeKind kind1 = LLVMGetTypeKind(dest_type);
	if (type == dest_type)
	{
		return val;
	}
	//目标类型是double
	if (dest_type == LLVMDoubleType())
	{
		if (llvm_is_int(val))
		{
			//bool类型需要特殊处理
			if (!llvm_is_bit(val))
				return LLVMBuildSIToFP(sem_ctx.builder, val, LLVMDoubleType(), "double_cast");
			else
				return LLVMBuildUIToFP(sem_ctx.builder, val, LLVMDoubleType(), "double_cast");
		}
		else
		{
			LLVMBuildFPCast(sem_ctx.builder, val, LLVMDoubleType(), "double_cast");
		}
	}
	//目标类型是float
	if (dest_type == LLVMFloatType())
	{
		if (llvm_is_int(val))
		{
			//bool类型需要特殊处理
			if (!llvm_is_bit(val))
				return LLVMBuildSIToFP(sem_ctx.builder, val, LLVMFloatType(), "float_cast");
			else
				return LLVMBuildUIToFP(sem_ctx.builder, val, LLVMFloatType(), "float_cast");
		}
		else
		{
			LLVMBuildFPTrunc(sem_ctx.builder, val, LLVMFloatType(), "float_cast");
		}
	}
	//目标是int64
	if (dest_type == LLVMInt64Type())
	{
		if (llvm_is_int(val))
		{
			if (!llvm_is_bit(val))
				return LLVMBuildSExtOrBitCast(sem_ctx.builder, val, LLVMInt64Type(), "int64_cast");
			else
				return LLVMBuildZExtOrBitCast(sem_ctx.builder, val, LLVMInt64Type(), "int64_cast");
		}
		else
		{
			return LLVMBuildFPToSI(sem_ctx.builder, val, LLVMInt64Type(), "int64_cast");
		}
	}
	//目标是int32
	else if (dest_type == LLVMInt32Type())
	{
		if (llvm_is_int(val))
		{
			if (!llvm_is_bit(val))
			{
				if (LLVMTypeOf(val) == LLVMInt64Type())
				{
					return LLVMBuildTrunc(sem_ctx.builder, val, LLVMInt32Type(), "int32_cast");
				}
				else
					return LLVMBuildSExtOrBitCast(sem_ctx.builder, val, LLVMInt32Type(), "int32_cast");
			}
			else
				return LLVMBuildZExtOrBitCast(sem_ctx.builder, val, LLVMInt32Type(), "int32_cast");
		}
		else
		{
			return LLVMBuildFPToSI(sem_ctx.builder, val, LLVMInt32Type(), "int32_cast");
		}
	}
	//目标是int8
	else if (dest_type == LLVMInt8Type())
	{
		if (llvm_is_int(val))
		{
			if (!llvm_is_bit(val))
			{
				return LLVMBuildTrunc(sem_ctx.builder, val, LLVMInt8Type(), "int8_cast");
			}
			else
				return LLVMBuildZExtOrBitCast(sem_ctx.builder, val, LLVMInt8Type(), "int8_cast");
		}
		else
			return LLVMBuildFPToSI(sem_ctx.builder, val, LLVMInt8Type(), "int8_cast");
	}
	//不支持的类型
	else
		return val;
}
// 这个函数主要确定二元运算符的结果类型
LLVMTypeRef llvm_get_res_type(LLVMValueRef lhs, LLVMValueRef rhs)
{
	LLVMTypeRef left_type = LLVMTypeOf(lhs);
	LLVMTypeRef right_type = LLVMTypeOf(rhs);
	//这儿由于函数没法用switch
	if (left_type == right_type)
		return left_type;
	else if (left_type == LLVMDoubleType() || right_type == LLVMDoubleType())
		return LLVMDoubleType();
	else if (left_type == LLVMFloatType() || right_type == LLVMFloatType())
		return LLVMFloatType();
	else if (left_type == LLVMInt64Type() || right_type == LLVMInt64Type())
		return LLVMInt64Type();
	else if (left_type == LLVMInt32Type() || right_type == LLVMInt32Type())
		return LLVMInt32Type();
	//暂时不支持int16
	//else if (left_type == LLVMInt16Type() || right_type == LLVMInt16Type())
	//	return LLVMInt16Type();
	else if (left_type == LLVMInt8Type() || right_type == LLVMInt8Type())
		return LLVMInt8Type();
	else if (left_type == LLVMInt1Type() || right_type == LLVMInt1Type())
		return LLVMInt1Type();
	else
		return LLVMInt32Type();
}

static LLVMValueRef eval_OP_ARRAY_ACCESS(OperatorExpr* op, int lv) {
	LLVMValueRef lhs_ptr = get_OperatorExpr_LeftValue(op->lhs);
	LLVMValueRef rhs_v = eval_OperatorExpr(op->rhs);
	LLVMValueRef* indices = calloc(2, sizeof(LLVMValueRef));
	indices[0] = LLVMConstInt(LLVMInt64Type(), 0, 1);
	indices[1] = rhs_v;
	LLVMValueRef gep = LLVMBuildGEP(sem_ctx.builder, lhs_ptr, indices, 2, "gep_res");
	if (lv) {
		return gep;
	}
	else {
		return LLVMBuildLoad(sem_ctx.builder, gep, "arr_res");
	}
}

/// <summary>
///  
/// </summary>
/// <param name="op"></param>
/// <param name="lv"></param>
/// <param name="stack_or_ptr">0: stack, 1: ptr</param>
/// <returns></returns>
static LLVMValueRef eval_OP_ACCESS(OperatorExpr* op, int lv, int stack_or_ptr) {
	LLVMValueRef lhs_ptr = get_OperatorExpr_LeftValue(op->lhs);
	if (stack_or_ptr) {
		lhs_ptr = LLVMBuildLoad(sem_ctx.builder, lhs_ptr, "load_res");
	}
	char* struct_name = LLVMGetStructName(LLVMGetElementType(LLVMTypeOf(lhs_ptr)));
	Symbol* struct_sym = symtbl_find(ctx->types, struct_name);
	TRY_CAST(IdentifierExpr, field_id, op->rhs);
	if (!field_id) {
		log_error(op, "rhs of OP_STACK_ACCESS expects IdentifierExpr");
		return NULL;
	}
	int idx = 0;
	TypeInfo* cur = struct_sym->type.struc.child;
	for (; cur != NULL; cur = cur->next, ++idx) {
		if (strcmp(cur->field_name, field_id->name) == 0) {
			break;
		}
	}

	if (cur == NULL) {
		log_error(op, "struct %s doesn't have field named %s", struct_name, field_id->name);
		return NULL;
	}

	LLVMValueRef gep = LLVMBuildStructGEP(sem_ctx.builder, lhs_ptr, idx, "gep_res");
	if (lv) {
		return gep;
	}
	else {
		return LLVMBuildLoad(sem_ctx.builder, gep, "struct_res");
	}
}

static LLVMValueRef eval_OP_POINTER(OperatorExpr* op, int lv) {
	LLVMValueRef lhs = eval_OperatorExpr(op->lhs);
	if (lv) {
		return lhs;
	}
	else {
		return LLVMBuildLoad(sem_ctx.builder, lhs, "ptr_res");
	}
}

// 临时加一个取左值的
LLVMValueRef get_OperatorExpr_LeftValue(AST* ast)
{
	if (ast->type == AST_IdentifierExpr)
	{
		TRY_CAST(IdentifierExpr, identifier, ast);
		if (!identifier)
		{
			log_error(ast, "Expected IdentifierExpr");
			return NULL;
		}
		Symbol* sym = symtbl_find(ctx->variables, identifier->name);
		if (sym == NULL) {
			return NULL;
		}
		return sym->value;
	}
	else if (ast->type == AST_OperatorExpr)
	{
		TRY_CAST(OperatorExpr, operator, ast);
		if (!operator)
		{
			log_error(ast, "Expected OperatorExpr");
			return NULL;
		}
		if (operator->op == OP_ARRAY_ACCESS) {
			return eval_OP_ARRAY_ACCESS(operator, 1);
		}
		else if (operator->op == OP_STACK_ACCESS) {
			return eval_OP_ACCESS(operator, 1, 0);
		}
		else if (operator->op == OP_PTR_ACCESS) {
			return eval_OP_ACCESS(operator, 1, 1);
		}
		else if (operator->op == OP_POINTER) {
			return eval_OP_POINTER(operator, 1);
		}
	}
	else if (ast->type == AST_ListExpr) {
		TRY_CAST(ListExpr, list, ast);
		return get_OperatorExpr_LeftValue(list->first_child);
	}
	return NULL;
}

// 目前仅考虑signed类型
LLVMValueRef eval_OperatorExpr(AST* ast)
{
	LLVMBasicBlockRef block;
	if (!ast) return NULL;
	LLVMValueRef lhs, rhs;
	LLVMTypeRef dest_type;
	LLVMTypeKind kind;
	if (ast->type == AST_NumberExpr)
	{
		TRY_CAST(NumberExpr, number, ast);
		if (!number)
		{
			log_error(ast, "Expected NumberExpr");
			return NULL;
		}
		return eval_NumberExpr(number);
	}
	else if (ast->type == AST_IdentifierExpr)
	{
		TRY_CAST(IdentifierExpr, identifier, ast);
		if (!identifier)
		{
			log_error(ast, "Expected IdentifierExpr");
			return NULL;
		}
		return get_identifierxpr_llvm_value(identifier);
	}
	else if (ast->type == AST_ListExpr) {
		TRY_CAST(ListExpr, list, ast);
		return eval_OperatorExpr(list->first_child);
	}
	else if (ast->type == AST_FunctionCallExpr)
	{
		return eval_FunctionCallExpr(ast);
	}
	else if (ast->type == AST_OperatorExpr)
	{
		TRY_CAST(OperatorExpr, operator, ast);
		if (!operator)
		{
			log_error(ast, "Expected OperatorExpr");
			return NULL;
		}
		// 好像确实加不到后面
		if (operator->op == OP_ARRAY_ACCESS) {
			// 一些比较特殊的op不适合和后面的一起做
			return eval_OP_ARRAY_ACCESS(operator, 0);
		}
		else if (operator->op == OP_STACK_ACCESS) {
			return eval_OP_ACCESS(operator, 0, 0);
		}
		else if (operator->op == OP_PTR_ACCESS) {
			return eval_OP_ACCESS(operator, 0, 1);
		}

		if (operator->op == OP_SIZEOF)
		{
			LLVMValueRef tmp = NULL;
			uint64_t result = 0;
			//TRY_CAST(IdentifierExpr, id, operator->lhs);
			//TRY_CAST(TypeSpecifier, spec, operator->lhs);
			TRY_CAST(DeclaratorExpr, decl, operator->lhs);
			if (decl != NULL)
			{
				if (decl->name != NULL)
				{
					Symbol* variable = symtbl_find(ctx->variables, decl->name);

					if (variable == NULL)
					{
						log_error(operator->lhs, "Undeclared identifier");
					}
					else {
						if (variable->var.type->type.incomplete)
						{
							log_error(operator->lhs, "Identifier's type is not declared or incomplete");
						}
						else {
							result = variable->var.type->type.aligned_size;
						}
					}
				}
				else
				{

					TypeInfo* ty = extract_type(decl->type_spec);
					if (ty == NULL || ty->incomplete)
					{
						log_error(operator->lhs, "Type specifier is incomplete");
					}
					result = ty->aligned_size;
				}
				tmp = LLVMConstInt(
					LLVMInt64Type(),
					result,
					0);
			}
			else {
				LLVMValueRef val = eval_ast(operator->lhs);
				if (val != NULL)
				{
					LLVMValueRef ty = LLVMTypeOf(val);
					tmp = LLVMSizeOf(ty);
				}
			}
			return tmp;
		}
		lhs = eval_OperatorExpr(operator->lhs);
		rhs = eval_OperatorExpr(operator->rhs);
		if (lhs && rhs && operator->op != OP_ASSIGN)
		{
			dest_type = llvm_get_res_type(lhs, rhs);
			kind = LLVMGetTypeKind(dest_type);
			lhs = llvm_convert_type(dest_type, lhs);
			rhs = llvm_convert_type(dest_type, rhs);
		}
		else
		{
			dest_type = LLVMTypeOf(lhs);
			kind = LLVMGetTypeKind(dest_type);
		}
		LLVMValueRef tmp = NULL, tmp1 = NULL, value_ptr = NULL;
		switch (operator->op)
		{
			// 一元运算符
		case OP_NEGATIVE:
			if (!llvm_is_float(lhs))
			{
				tmp = LLVMBuildNeg(sem_ctx.builder, lhs, "neg_res");
			}
			else
			{
				tmp = LLVMBuildFNeg(sem_ctx.builder, lhs, "fneg_res");
			}
			break;
		case OP_COMPLEMENT:
			if (!llvm_is_float(lhs))
			{
				tmp = LLVMBuildNot(sem_ctx.builder, lhs, "not_res");
			}
			else
			{
				log_error(ast, "COMPLEMENT Op Expected INT Type");
			}
			break;
		case OP_POINTER:
			tmp = LLVMBuildLoad(sem_ctx.builder, lhs, "ptr_res");
			break;
		case OP_INC:
			if (!llvm_is_float(lhs))
			{
				tmp = LLVMBuildAdd(sem_ctx.builder, lhs, LLVMConstInt(dest_type, 1, 1), "prefix_inc_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp, value_ptr);
			}
			else
			{
				tmp = LLVMBuildFAdd(sem_ctx.builder, lhs, LLVMConstReal(dest_type, 1), "prefix_finc_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp, value_ptr);
			}
			break;
		case OP_DEC:
			if (!llvm_is_float(lhs))
			{
				tmp = LLVMBuildAdd(sem_ctx.builder, lhs, LLVMConstInt(dest_type, -1, 1), "prefix_dec_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp, value_ptr);
			}
			else
			{
				tmp = LLVMBuildFAdd(sem_ctx.builder, lhs, LLVMConstReal(dest_type, -1), "prefix_fdec_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp, value_ptr);
			}
			break;
		case OP_POSTFIX_INC:
			if (!llvm_is_float(lhs))
			{
				tmp = lhs;
				tmp1 = LLVMBuildAdd(sem_ctx.builder, lhs, LLVMConstInt(dest_type, 1, 1), "postfix_inc_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp1, value_ptr);
			}
			else
			{
				tmp = lhs;
				tmp1 = LLVMBuildFAdd(sem_ctx.builder, lhs, LLVMConstReal(dest_type, 1), "postfix_finc_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp1, value_ptr);
			}
			break;
		case OP_POSTFIX_DEC:
			if (!llvm_is_float(lhs))
			{
				tmp = lhs;
				tmp1 = LLVMBuildAdd(sem_ctx.builder, lhs, LLVMConstInt(dest_type, -1, 1), "postfix_dec_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp1, value_ptr);
			}
			else
			{
				tmp = lhs;
				tmp1 = LLVMBuildFAdd(sem_ctx.builder, lhs, LLVMConstReal(dest_type, -1), "postfix_fdec_res");
				value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
				LLVMBuildStore(sem_ctx.builder, tmp1, value_ptr);
			}
			break;
			// 二元运算符
		case OP_ADD:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildAdd(sem_ctx.builder, lhs, rhs, "add_res");
			}
			else
			{
				tmp = LLVMBuildFAdd(sem_ctx.builder, lhs, rhs, "fadd_res");
			}
			break;
		case OP_SUB:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildSub(sem_ctx.builder, lhs, rhs, "dec_res");
			}
			else
			{
				tmp = LLVMBuildFSub(sem_ctx.builder, lhs, rhs, "fdec_res");
			}
			break;
		case OP_MUL:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildMul(sem_ctx.builder, lhs, rhs, "mul_res");
			}
			else
			{
				tmp = LLVMBuildFMul(sem_ctx.builder, lhs, rhs, "fmul_res");
			}
			break;
		case OP_DIV:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildExactSDiv(sem_ctx.builder, lhs, rhs, "div_res"); //默认为signed了 有需求再改吧
			}
			else
			{
				tmp = LLVMBuildFDiv(sem_ctx.builder, lhs, rhs, "fdiv_res");
			}
			break;
		case OP_MOD:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildSRem(sem_ctx.builder, lhs, rhs, "mod_res");
			}
			else
			{
				tmp = LLVMBuildFRem(sem_ctx.builder, lhs, rhs, "fmod_res");
			}
			break;
		case OP_SHIFT_LEFT:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildShl(sem_ctx.builder, lhs, rhs, "sll_res");
			}
			else
			{
				log_error(ast, "SHIFT Op Expected INT Type");
			}
			break;
		case OP_SHIFT_RIGHT:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildAShr(sem_ctx.builder, lhs, rhs, "sra_res"); //算数移位
			}
			else
			{
				log_error(ast, "SHIFT Op Expected INT Type");
			}
			break;
			//位运算
		case OP_BIT_AND:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildAnd(sem_ctx.builder, lhs, rhs, "and_res");
			}
			else
			{
				log_error(ast, "And Op Expected INT Type");
			}
			break;
		case OP_BIT_OR:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildOr(sem_ctx.builder, lhs, rhs, "or_res");
			}
			else
			{
				log_error(ast, "BIT_OR Op Expected INT Type");
			}
			break;
		case OP_BIT_XOR:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildXor(sem_ctx.builder, lhs, rhs, "xor_res");
			}
			else
			{
				log_error(ast, "BIT_XOR Op Expected INT Type");
			}
			break;
		case OP_AND:
			if (!type_is_float(dest_type))
			{
				lhs = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, lhs, LLVMConstInt(dest_type, 0, 1), "equal_res");
				rhs = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, rhs, LLVMConstInt(dest_type, 0, 1), "equal_res");
				tmp = LLVMBuildAnd(sem_ctx.builder, lhs, rhs, "and_res");
			}
			else
			{
				log_error(ast, "And Op Expected INT Type");
			}
			break;
		case OP_OR:
			if (!type_is_float(dest_type))
			{
				lhs = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, lhs, LLVMConstInt(dest_type, 0, 1), "equal_res");
				rhs = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, rhs, LLVMConstInt(dest_type, 0, 1), "equal_res");
				tmp = LLVMBuildOr(sem_ctx.builder, lhs, rhs, "or_res");
			}
			else
			{
				log_error(ast, "SHIFT Op Expected INT Type");
			}
			break;
			//比较运算符返回的是INT_1类型
		case OP_LESS:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildICmp(sem_ctx.builder, LLVMIntSLT, lhs, rhs, "less_res");
			}
			else
				tmp = LLVMBuildFCmp(sem_ctx.builder, LLVMRealOLT, lhs, rhs, "less_res");
			break;
		case OP_LESS_OR_EQUAL:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildICmp(sem_ctx.builder, LLVMIntSLE, lhs, rhs, "less_equal_res");
			}
			else
				tmp = LLVMBuildFCmp(sem_ctx.builder, LLVMRealOLE, lhs, rhs, "less_equal_res");
			break;
		case OP_GREATER:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildICmp(sem_ctx.builder, LLVMIntSGT, lhs, rhs, "greater_res");
			}
			else
				tmp = LLVMBuildFCmp(sem_ctx.builder, LLVMRealOGT, lhs, rhs, "greater_res");
			break;
		case OP_GREATER_OR_EQUAL:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildICmp(sem_ctx.builder, LLVMIntSGE, lhs, rhs, "greater_equal_res");
			}
			else
				tmp = LLVMBuildFCmp(sem_ctx.builder, LLVMRealOGE, lhs, rhs, "greater_equal_res");
			break;
		case OP_EQUAL:
			if (!type_is_float(dest_type))
			{
				tmp = LLVMBuildICmp(sem_ctx.builder, LLVMIntEQ, lhs, rhs, "equal_res");
			}
			else
				tmp = LLVMBuildFCmp(sem_ctx.builder, LLVMRealOEQ, lhs, rhs, "equal_res");
			break;
		case OP_NOT_EQUAL:
			if (!type_is_float(dest_type)) {
				tmp = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, lhs, rhs, "equal_res");
			}
			else
				tmp = LLVMBuildFCmp(sem_ctx.builder, LLVMRealONE, lhs, rhs, "equal_res");
			break;
			//赋值运算符
		case OP_ASSIGN:
			dest_type = LLVMTypeOf(lhs);
			rhs = llvm_convert_type(dest_type, rhs);
			value_ptr = get_OperatorExpr_LeftValue(operator->lhs);
			LLVMBuildStore(sem_ctx.builder, rhs, value_ptr);
			tmp = rhs;
			break;
		default:
			return NULL;
			break;
		}
		return tmp;
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

// @wushuhu
void eval_WhileStmt(LoopStmt* ast) {
	LLVMBasicBlockRef preheader_bb = LLVMGetInsertBlock(sem_ctx.builder);
	LLVMBasicBlockRef cond_bb = alloc_bb("while.cond");
	LLVMBasicBlockRef body_bb = alloc_bb("while.body");
	LLVMBasicBlockRef after_bb = alloc_bb("while.after");

	// 1. preheader
	LLVMBuildBr(sem_ctx.builder, cond_bb);

	// 2. cond
	LLVMPositionBuilderAtEnd(sem_ctx.builder, cond_bb);

	LLVMValueRef condv = eval_ast(ast->condition);
	if (condv == NULL) {
		log_error(ast, "while.cond is not expr");
		return NULL;
	}
	else {
		if (llvm_is_float(condv)) {
			fprintf(stderr, "Cond is float, which is not allowed. Implicit converted to true\n");
			LLVMBuildBr(sem_ctx.builder, body_bb);
		}
		else {
			condv = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, condv, LLVMConstInt(LLVMTypeOf(condv), 0, 1), next_temp_id_str());
			LLVMBuildCondBr(sem_ctx.builder, condv, body_bb, after_bb);
		}
	}

	append_bb(&sem_ctx.breakable_last, after_bb);
	append_bb(&sem_ctx.continue_last, cond_bb);
	append_bb(&sem_ctx.after_bb, after_bb);
	// 3. body
	LLVMPositionBuilderAtEnd(sem_ctx.builder, body_bb);
	LLVMValueRef bodyv = eval_BlockExprNoScope(ast->body);
	if (!llvm_is_b(bodyv)) {	// 如果最后一条指令是br，不能构建br
		LLVMBuildBr(sem_ctx.builder, cond_bb);
	}

	// 4. after
	pop_bb(&sem_ctx.breakable_last);
	pop_bb(&sem_ctx.continue_last);
	pop_bb(&sem_ctx.after_bb);
	LLVMPositionBuilderAtEnd(sem_ctx.builder, after_bb);
}

// @wushuhui
void eval_ForStmt(LoopStmt* ast) {
	ctx_enter_block_scope(ctx);

	LLVMValueRef enter = eval_ast(ast->enter);

	LLVMBasicBlockRef preheader_bb = LLVMGetInsertBlock(sem_ctx.builder);
	LLVMBasicBlockRef cond_bb = alloc_bb("for.cond");
	LLVMBasicBlockRef body_bb = alloc_bb("for.body");
	LLVMBasicBlockRef step_bb = alloc_bb("for.step");
	LLVMBasicBlockRef after_bb = alloc_bb("for.after");

	// 1. preheader
	LLVMBuildBr(sem_ctx.builder, cond_bb);

	// 2. cond
	LLVMPositionBuilderAtEnd(sem_ctx.builder, cond_bb);
	LLVMValueRef condv = eval_ast(ast->condition);
	if (condv != NULL) {
		if (llvm_is_float(condv)) {
			fprintf(stderr, "Cond is float, which is not allowed. Implicit converted to true\n");
			LLVMBuildBr(sem_ctx.builder, body_bb);
		}
		else {
			condv = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, condv, LLVMConstInt(LLVMTypeOf(condv), 0, 1), next_temp_id_str());
			LLVMBuildCondBr(sem_ctx.builder, condv, body_bb, after_bb);
		}
	}
	else {
		// for循环是允许cond为空的
		LLVMBuildBr(sem_ctx.builder, body_bb);
	}

	append_bb(&sem_ctx.breakable_last, after_bb);
	append_bb(&sem_ctx.continue_last, step_bb);
	append_bb(&sem_ctx.after_bb, after_bb);
	// 3. body
	LLVMPositionBuilderAtEnd(sem_ctx.builder, body_bb);
	LLVMValueRef bodyv = eval_ast(ast->body);
	if (!llvm_is_b(bodyv)) {	// 如果最后一条指令是br，不能构建br
		LLVMBuildBr(sem_ctx.builder, step_bb);
	}

	// 4. step
	LLVMPositionBuilderAtEnd(sem_ctx.builder, step_bb);
	LLVMValueRef step = eval_ast(ast->step);
	if (step == NULL) {
		return NULL;		// FIX
	}
	LLVMBuildBr(sem_ctx.builder, cond_bb);

	// 5. after
	ctx_leave_block_scope(ctx, 0);
	pop_bb(&sem_ctx.breakable_last);
	pop_bb(&sem_ctx.continue_last);
	pop_bb(&sem_ctx.after_bb);
	LLVMPositionBuilderAtEnd(sem_ctx.builder, after_bb);
}

// @wushuhui
LLVMValueRef eval_LoopStmt(LoopStmt* ast) {
	switch (ast->loop_type) {
	case LOOP_FOR:
		eval_ForStmt(ast);
		break;
	case LOOP_WHILE:
		eval_WhileStmt(ast);
		break;
	default:
		// 不支持do while
		break;
	}
	return NULL;
}
//增加临时测试代码的地方，贫穷.jpg
void sentence_test()
{
	LLVMValueRef v1 = LLVMConstInt(LLVMInt32Type(), 10, 1);
	LLVMValueRef v2 = LLVMConstInt(LLVMInt32Type(), 10, 1);
	LLVMValueRef v3 = LLVMBuildFNeg(sem_ctx.builder, v2, "test");
	LLVMValueRef v4 = LLVMBuildAlloca(sem_ctx.builder, LLVMFloatType(), "test");
	LLVMBuildStore(sem_ctx.builder, v3, v4);
	//LLVMValueRef v7 = LLVMBuildSIToFP(sem_ctx.builder, v1, LLVMFloatType(), "float_cast");
	//LLVMValueRef v5 = LLVMBuildStore(sem_ctx.builder, v2, v3);
}

// @wushuhui
LLVMValueRef eval_IfStmt(IfStmt* ast) {
	LLVMValueRef condv = eval_ast(ast->condition);
	if (condv == NULL) {
		log_error(ast, "if.cond is not expr");
		return NULL;
	}
	LLVMBasicBlockRef then_bb = alloc_bb("if.then");
	LLVMBasicBlockRef else_bb = alloc_bb("if.else");
	LLVMBasicBlockRef after_bb = alloc_bb("if.after");

	// 似乎clang的标准并不允许double作为条件的值，会有下述warning：
	// implicit conversion from 'double' to '_Bool' changes value from 1.111 to true
	if (llvm_is_float(condv)) {
		fprintf(stderr, "Cond is float, which is not allowed. Implicit converted to true\n");
		LLVMBuildBr(sem_ctx.builder, then_bb);
	}
	else {
		condv = LLVMBuildICmp(sem_ctx.builder, LLVMIntNE, condv, LLVMConstInt(LLVMTypeOf(condv), 0, 1), next_temp_id_str());
		LLVMBuildCondBr(sem_ctx.builder, condv, then_bb, else_bb);
	}

	LLVMPositionBuilderAtEnd(sem_ctx.builder, then_bb);
	//sentence_test(); // 1号测试坑
   // 有可能then里面没有东西
	LLVMValueRef innerv = NULL;
	if (!ast->then) {
		log_error(ast, "if.then is empty");
		return NULL;
	}
	else if (ast->then->type != AST_EmptyExpr) {
		append_bb(&sem_ctx.after_bb, else_bb);
		innerv = eval_ast(ast->then);
		pop_bb(&sem_ctx.after_bb);
	}
	if (innerv == NULL || (innerv != NULL && !llvm_is_b(innerv))) {	// 如果最后一条指令是br，不能构建br
		LLVMBuildBr(sem_ctx.builder, after_bb);
	}

	LLVMPositionBuilderAtEnd(sem_ctx.builder, else_bb);
	innerv = NULL;
	// otherwise不一定有, clang有一个优化，如果没有otherwise就不生成这个BB，这里为了方便也生成了(代码大小问题不是问题)
	if (ast->otherwise && ast->otherwise->type != AST_EmptyExpr) {
		append_bb(&sem_ctx.after_bb, else_bb);
		innerv = eval_ast(ast->otherwise);
		pop_bb(&sem_ctx.after_bb);
	}
	if (innerv == NULL || (innerv != NULL && !llvm_is_b(innerv))) {	// 如果最后一条指令是br，不能构建br
		LLVMBuildBr(sem_ctx.builder, after_bb);
	}

	LLVMPositionBuilderAtEnd(sem_ctx.builder, after_bb);

	return NULL;
}

// TODO: @wushuhui 优先级低，暂时先不实现
LLVMValueRef eval_SwitchCaseStmt(SwitchCaseStmt* ast) {
	//if (ast->cases == NULL) {
	//	log_error(ast, "switch.body should not be empty");
	//	return NULL;
	//}
	//ctx_enter_block_scope(ctx);
	//LLVMValueRef condv = eval_ast(ast->switch_value);
	//if (condv == NULL) {
	//	log_error(ast, "switch.cond is not expr");
	//	return NULL;
	//}
	//LLVMValueRef func = LLVMGetBasicBlockParent(LLVMGetInsertBlock(sem_ctx.builder));
	//LLVMBasicBlockRef else_bb = LLVMAppendBasicBlock(func, "else");
	//AST* case_ast = ((BlockExpr*)ast->cases)->first_child;
	//unsigned int casec = 0;
	//for (; case_ast != NULL; case_ast = case_ast->next, ++casec);
	//LLVMValueRef switchv = LLVMBuildSwitch(sem_ctx.builder, condv, else_bb, casec);
	//append_bb(&sem_ctx.breakable_last, else_bb);
	//append_bb(&sem_ctx.after_bb, else_bb);

	//char* last_id;
	//for (case_ast = ((BlockExpr*)ast->cases)->first_child; case_ast != NULL; case_ast = case_ast->next) {
	//	TRY_CAST(LabelStmt, case_stmt, case_ast);
	//	if (case_stmt == NULL) {
	//		continue;
	//	} else if (case_stmt->type == LABEL_CASE) {
	//		// 
	//	} else if (case_stmt->type == LABEL_DEFAULT) {

	//	} else {
	//		continue;
	//	}
	//	last_id = next_temp_id_str();
	//	LLVMBasicBlockRef case_bb = alloc_bb(last_id);
	//	LLVMPositionBuilderAtEnd(sem_ctx.builder, case_bb);
	//	eval_ast(case_stmt->target);
	//	// TODO 理论上cond只能是可以计算出来的常数
	//	LLVMAddCase(switchv, eval_ast(case_stmt->condition), case_bb);
	//}

	//pop_bb(&sem_ctx.breakable_last);
	//pop_bb(&sem_ctx.after_bb);
	//ctx_leave_block_scope(ctx, 0);

}

// @wushuhui
LLVMValueRef eval_FunctionDefinitionStmt(FunctionDefinitionStmt* ast) {
	TRY_CAST(DeclaratorExpr, decl_ast, ast->declarator);
	if (decl_ast == NULL) {
		return NULL;
	}
	TRY_CAST(TypeSpecifier, type_ast, ast->specifier);
	if (type_ast == NULL) {
		return NULL;
	}

	LLVMValueRef func;

	Symbol* func_sym = symtbl_find(ctx->functions, decl_ast->name);
	if (func_sym == NULL) {
		// 没有声明，那么定义和声明在一起
		func_sym = eval_FuncDeclareStmt(ast, type_ast, decl_ast->name, decl_ast->type_spec->params);
		func_sym->func.body = ast;
		func = func_sym->func.value;
	}
	else if (func_sym->func.body != NULL) {
		// 已经有定义了
		log_error(SUPER(ast), "Function Redifinition");
		return NULL;
	}
	else {
		// 如果符号表中已经有了函数，那么用那个符号
		func = func_sym->value;
		func_sym->func.body = ast->body;
		// TODO 检查形参和实参是否一致
	}
	sem_ctx.cur_func_sym = func_sym;

	ctx_enter_function_scope(ctx);
	enter_sematic_temp_context();

	LLVMBasicBlockRef entry_bb = LLVMAppendBasicBlock(func, "entry");
	LLVMPositionBuilderAtEnd(sem_ctx.builder, entry_bb);

	// 构建实参
	unsigned int n_params = LLVMCountParams(func);
	LLVMValueRef* params = malloc(sizeof(LLVMValueRef) * n_params);
	LLVMGetParams(func, params);
	TypeSpecifier* arg_typesp = decl_ast->type_spec->params;
	for (int i = 0; i < n_params; ++i) {
		LLVMSetValueName(params[i], arg_typesp->field_name);	// 给参数一个名字
		// 为参数分配栈空间并store
		LLVMValueRef* ptr = LLVMBuildAlloca(sem_ctx.builder, func_sym->func.params[i], next_temp_id_str());
		LLVMBuildStore(sem_ctx.builder, params[i], ptr);
		Symbol* arg_sym = symbol_create_variable(arg_typesp->field_name, arg_typesp->attributes, symbol_from_type_info(arg_typesp), ptr, 0);
		symtbl_push(ctx->variables, arg_sym);
		arg_typesp = arg_typesp->super.next;
	}

	// NOTE: 这里不能直接用Block，要hack一下。FunctionDef的时候是会创建一个scope的
	//		如果function的body自己又创建一个scope，
	//		而在body内定义变量的时候只检查current scope内有没有重名，也就是说会出现body内定义的变量覆盖函数实参的情况
	LLVMValueRef bodyv = eval_BlockExprNoScope(ast->body);

	// RET
	// 如果前面的指令没有return，需要额外构建return
	if (bodyv == NULL || !LLVMIsAReturnInst(bodyv)) {
		if (func_sym->func.ret_type == LLVMVoidType()) {
			LLVMBuildRetVoid(sem_ctx.builder);
		}
		else if (llvm_is_float(func_sym->func.ret_type)) {
			LLVMBuildRet(sem_ctx.builder, LLVMConstReal(func_sym->func.ret_type, 0));
		}
		else {
			LLVMBuildRet(sem_ctx.builder, LLVMConstInt(func_sym->func.ret_type, 0, 1));
		}
	}


	sem_ctx.cur_func_sym = NULL;
	leave_sematic_temp_context();
	ctx_leave_function_scope(ctx);
	return NULL;
}

// 这些应该在更上层的函数里处理掉
LLVMValueRef eval_DeclaratorExpr(DeclaratorExpr* ast)
{
	NOT_IMPLEMENTED;
}

LLVMValueRef eval_TypeSpecifier(TypeSpecifier* ast)
{
	NOT_IMPLEMENTED;


}
/*
LLVMValueRef handle_initializer_list(TypeInfo* info, ListExpr* ast)
{

}

*/