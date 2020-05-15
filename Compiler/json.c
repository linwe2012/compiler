#include "json.h"

#include <stdio.h>

#define PRINT_NAME(n) fputs("\"name\":\"" #n "\"", s_fp);

FILE* s_fp = NULL;

void static _write_ast(AST* root);
#define AST_FUNCDECL(x) void static _write_##x(x*);
AST_NODE_LIST(AST_FUNCDECL)
#undef AST_FUNCDECL

/******************************************************/

void ast_to_json(AST* root, const char* filepath) {
    s_fp = fopen(filepath, "w");
    _write_ast(root);
    fclose(s_fp);
}

/*****************************************************/

void _write_ast(AST* root) {
    fputs("{\n", s_fp);

#define AST_WRITE_CASE(x) case AST_##x: _write_##x((x*)root); break;
    switch (root->type) {
		AST_NODE_LIST(AST_WRITE_CASE)
	default:
		break;
	}
#undef AST_WRITE_CASE

    fputs("\n}", s_fp);

    if (root->next) {
        // 兄弟节点
        fputs(",\n", s_fp);
        _write_ast(root->next);
    }
}

void _write_EmptyExpr(EmptyExpr* expr)
{
    return;
}

void _write_BlockExpr(BlockExpr* expr) {
    PRINT_NAME(BLOCK);
    fputs(",\n\"children\": [", s_fp);
    _write_ast((expr->first_child);
    fputs("]\n", s_fp);
}

void _write_OperatorExpr(OperatorExpr* expr) {
    switch (expr->op) {
    case OP_INC:
        PRINT_NAME(++); break;
    case OP_DEC:
        PRINT_NAME(--); break;
    case OP_UNARY_STACK_ACCESS:
        PRINT_NAME(.(access)); break;
    case OP_POSTFIX_INC:
        PRINT_NAME(++(postfix)); break;
    case OP_POSTFIX_DEC:
        PRINT_NAME(--(postfix)); break;
    case OP_POINTER:
        PRINT_NAME(*(indirection)); break;
    case OP_ADDRESS:
        PRINT_NAME(&); break;
    case OP_COMPLEMENT:
        PRINT_NAME(~); break;
    case OP_NOT:
        PRINT_NAME(!); break;
    // TODO 所有的op
    default:
        break;
    }
    fputs(",\n\"children\": [", s_fp);
    _write_ast(expr->first_child);
    fputs("]\n", s_fp);
}