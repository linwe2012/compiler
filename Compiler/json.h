#pragma once

#include "ast.h"

/// 将AbstractSyntaxTree转换为json文件
void ast_to_json(AST* root, const char* filepath);