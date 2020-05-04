#pragma once
#include "ast.h"
#include <assert.h>
//void log_error(AST* node, const char* message);
//void log_warning(AST* node, const char* message);

void cc_log_error(const char* file, int line, AST* ast, const char* message, ...);

// TODO
#define log_error(ast_node, message, ...) cc_log_error( __FILE__, __LINE__, ast_node, message, __VA_ARGS__)
#define log_warning(ast_node, message, ...) cc_log_error( __FILE__, __LINE__, ast_node, message, __VA_ARGS__)
#define log_internal_error(ast_node, message, ...) cc_log_error( __FILE__, __LINE__, ast_node, message, __VA_ARGS__)

#include <assert.h>
#define CC_ASSERT(x,  ast_node, message) assert(x)