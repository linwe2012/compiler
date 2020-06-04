#include "config.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "context.h"
#include "x86_64-asm.h"
#include "json.h"
extern AST* parser_result;

extern int yydebug;


int main()
{
	Context ctx;
	symbol_init_context(&ctx);
	ast_init_context(&ctx);

#ifdef CC_DEBUG
	// parser_set_debug(0);
	// yydebug = 1;
#endif
	yyin = fopen("test/mini.c", "rt");
	int res = yyparse();
	if (res == 0)
	{
		printf("Parser success\n");
	}
	else {
		printf("Parser failed\n");
	}
	// AST* ast = make_block(parser_result);// yylval.val;
	ast_to_json(parser_result, "test/out.json");

	do_eval(parser_result, &ctx, "mini.c");

	printf("done");
	return res;
}