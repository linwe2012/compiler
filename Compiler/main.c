#include "config.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "context.h"
#include "x86_64-asm.h"
#include "json.h"
#include "yacc-error.h"

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
	// yydebug = 1;
#endif
	yyin = fopen("test/mini.c", "rt");
	char b[120];
	fgets(b, 120, yyin);
	yyin = NULL;
	SetFile(fopen("test/mini.c", "rt"));
	int res = yyparse();
	if (res == 0)
	{
		printf("Parser success\n");
	}
	else {
		printf("Parser failed\n");
		return 1;
	}
	if(error_encountered)
	{
		printf("Error encountered, will not generate ir\n");
		return 1;
	}
	// AST* ast = make_block(parser_result);// yylval.val;
	ast_to_json(parser_result, "test/out.json");

	do_eval(parser_result, &ctx, "mini.c", "test/mini.ll");

	printf("done");
	return res;
}