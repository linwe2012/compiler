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


void _write_BlockExpr(BlockExpr* expr) {
    PRINT_NAME(Block);
    fputs(",\n\"children\": [", s_fp);
    _write_ast(expr->first_child);
    fputs("]\n", s_fp);
}

void _write_ListExpr(ListExpr* expr) {
    PRINT_NAME(List);
    fputs(",\n\"children\": [", s_fp);
    _write_ast(expr->first_child);
    fputs("]\n", s_fp);
}

void _write_FunctionCallExpr(FunctionCallExpr* expr) {
    fprintf(s_fp, "\"name\":\"%s()\"", expr->function_name);
    fputs(",\n\"children\": [", s_fp);
    _write_ast(expr->params);
    fputs("]\n", s_fp);
}

void _write_IdentifierExpr(IdentifierExpr* expr) {
    fprintf(s_fp, "\"name\":\"%s\"", expr->name);
}

void _write_NumberExpr(NumberExpr* expr) {
    PRINT_NAME(Number);
    // switch (expr->number_type) {
    // // Types
    // default:
    //     PRINT_NAME(UNK); break;
    // }
}

void _write_SymbolExpr(SymbolExpr* expr) {
}

void _write_OperatorExpr(OperatorExpr* expr) {
    int has_child = 0;
    switch (expr->op) {
    case OP_INC:
        PRINT_NAME(++); break;
    case OP_DEC:
        PRINT_NAME(--); break;
    case OP_UNARY_STACK_ACCESS:
        PRINT_NAME(.(unary)); break;
    case OP_POSTFIX_INC:
        PRINT_NAME(++(postfix)); break;
    case OP_POSTFIX_DEC:
        PRINT_NAME(--(postfix)); break;
    case OP_POINTER:
        PRINT_NAME(*(deref)); break;
    case OP_ADDRESS:
        PRINT_NAME(&(addr)); break;
    case OP_COMPLEMENT:
        PRINT_NAME(~); break;
    case OP_NOT:
        PRINT_NAME(!); break;
    case OP_POSITIVE:
        PRINT_NAME(+(unary)); break;
    case OP_NEGATIVE:
        PRINT_NAME(-(unary)); break;
    case OP_SIZEOF:
        PRINT_NAME(sizeof); break;
    case OP_CAST:
        PRINT_NAME(cast); break;
    case OP_BIT_AND:
        PRINT_NAME(&(logic)); break;
    case OP_BIT_OR:
        PRINT_NAME(|); break;
    case OP_BIT_XOR:
        PRINT_NAME(^); break;
    case OP_MUL:
        PRINT_NAME(*(mul)); break;
    case OP_DIV:
        PRINT_NAME(/); break;
    case OP_MOD:
        PRINT_NAME(%); break;
    case OP_ADD:
        PRINT_NAME(+(binary)); break;
    case OP_SUB:
        PRINT_NAME(-(binary)); break;
    case OP_SHIFT_LEFT:
        PRINT_NAME(<<); break;
    case OP_SHIFT_RIGHT:
        PRINT_NAME(>>); break;
    case OP_ASSIGN:
        PRINT_NAME(=); break;
    case OP_ASSIGN_SHL:
        PRINT_NAME(<<=); break;
    case OP_ASSIGN_SHR:
        PRINT_NAME(>>=); break;
    case OP_ASSIGN_ADD:
        PRINT_NAME(+=); break;
    case OP_ASSIGN_SUB:
        PRINT_NAME(-=); break;
    case OP_ASSIGN_MUL:
        PRINT_NAME(*=); break;
    case OP_ASSIGN_DIV:
        PRINT_NAME(/=); break;
    case OP_ASSIGN_AND:
        PRINT_NAME(&=); break;
    case OP_ASSIGN_XOR:
        PRINT_NAME(^=); break;
    case OR_ASSIGN_OR:
        PRINT_NAME(|=); break;
    case OP_EQUAL:
        PRINT_NAME(==); break;
    case OP_NOT_EQUAL:
        PRINT_NAME(!=); break;
    case OP_LESS:
        PRINT_NAME(<); break;
    case OP_LESS_OR_EQUAL:
        PRINT_NAME(<=); break;
    case OP_GREATER:
        PRINT_NAME(>); break;
    case OP_GREATER_OR_EQUAL:
        PRINT_NAME(>=); break;
    case OP_AND:
        PRINT_NAME(&&); break;
    case OP_OR:
        PRINT_NAME(||); break;
    case OP_STACK_ACCESS:
        PRINT_NAME(.(binary)); break;
    case OP_PTR_ACCESS:
        PRINT_NAME(->); break;
    case OP_ARRAY_ACCESS:
        PRINT_NAME([]); break;
    case OP_CONDITIONAL:
        PRINT_NAME(?:); break;
    default:
        PRINT_NAME(UNK); break;
    }
    fputs(",\n\"children\": [", s_fp);
    if (expr->cond) {
        _write_ast(expr->cond);
        has_child = 1;
    }
    if (expr->lhs) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(expr->lhs);
        has_child = 1;
    }
    if (expr->rhs) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(expr->rhs);
    }
    fputs("]\n", s_fp);
}

void _write_LabelStmt(LabelStmt* stmt) {
    fprintf(s_fp, "\"name\":\"%s\"", stmt->ref->label->name);
    fputs(",\n\"children\": [", s_fp);
    if (stmt->condition) {
        _write_ast(stmt->condition);
    }
    // 需要打印target吗？还是说target只是AST分析中的辅助信息
    // leon> 要的
    if (stmt->target) {
        fputs(",\n", s_fp);
        _write_ast(stmt->target);
    }
    fputs("]\n", s_fp);
}

void _write_JumpStmt(JumpStmt* stmt) {
    switch (stmt->type) {
    case JUMP_GOTO:
        PRINT_NAME(Goto); break;
    case JUMP_BREAK:
        PRINT_NAME(Break); break;
    case JUMP_CONTINUE:
        PRINT_NAME(Continue); break;
    case JUMP_RET:
        PRINT_NAME(Return); break;
    default:
        PRINT_NAME(UNK); break;
    }
}

void _write_InitilizerListExpr(InitilizerListExpr* expr) {
    PRINT_NAME(InitList);
    if (expr->list) {
        fputs(",\n\"children\": [", s_fp);
        _write_ast(expr->list);
        fputs("]\n", s_fp);
    }
}

void _write_EmptyExpr(EmptyExpr* expr) {
    return;
}

void _write_TypenameExpr(TypenameExpr* expr) {
    // 2020-05-25 未定义
    fprintf(s_fp, "\"name\":\"UNK\"");
}

void _write_LoopStmt(LoopStmt* stmt) {
    switch (stmt->loop_type) {
    case LOOP_DOWHILE:
        PRINT_NAME(DoWhile); break;
    case LOOP_FOR:
        PRINT_NAME(For); break;
    case LOOP_WHILE:
        PRINT_NAME(While); break;
    default:
        PRINT_NAME(UNK); break;
    }
    fputs(",\n\"children\": [", s_fp);
    int has_child = 0;
    if (stmt->enter) {
        _write_ast(stmt->enter);
        has_child = 1;
    }
    if (stmt->condition) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(stmt->condition);
        has_child = 1;
    }
    if (stmt->step) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(stmt->step);
        has_child = 1;
    }
    if (stmt->body) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(stmt->body);
    }
    fputs("]\n", s_fp);
}

void _write_IfStmt(IfStmt* stmt) {
    PRINT_NAME(If);
    fputs(",\n\"children\": [", s_fp);
    int has_child = 0;
    if (stmt->condition) {
        _write_ast(stmt->condition);
        has_child = 1;
    }
    if (stmt->then) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(stmt->then);
        has_child = 1;
    }
    if (stmt->otherwise) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(stmt->otherwise);
    }
    fputs("]\n", s_fp);
}

void _write_SwitchCaseStmt(SwitchCaseStmt* stmt) {
    PRINT_NAME(Switch);
    fputs(",\n\"children\": [", s_fp);
    int has_child = 0;
    if (stmt->switch_value) {
        _write_ast(stmt->switch_value);
        has_child = 1;
    }
    if (stmt->cases) {
        if (has_child) {
            fputs(",\n", s_fp);
        }
        _write_ast(stmt->cases);
    }
    // Symbol* exit_label;是什么？
    fputs("]\n", s_fp);
}

void _write_DeclaratorExpr(DeclaratorExpr* expr) {
    fprintf(s_fp, "\"name\":\"%s(decl)\"", expr->name);
    fputs(",\n\"children\": [", s_fp);
    if (expr->init_value) {
        _write_ast(expr->init_value);
    }
    fputs("]\n", s_fp);
}

void _write_TypeSpecifier(TypeSpecifier* expr) {
    fprintf(s_fp, "\"name\":\"%s(decl)\"", expr->name);
}

