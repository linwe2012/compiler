#include "config.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"

#include "x86_64-asm.h"

#include <windows.h>   // WinApi header
extern AST* parser_result;

void yyerror(char const* s) {

	

	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	fprintf(stderr, "%s\n", s);
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	
}

int main()
{
#ifdef CC_DEBUG
	parser_set_debug(0);
#endif
	yyin = fopen("test/mini.c", "rt");
	int res = yyparse();
	ASMContext asm_ctx = {
		.assembly = 1
	};
	if (res == 0)
	{
		printf("Parser success\n");
	}
	else {
		printf("Parser failed\n");
	}
	AST* ast = make_block(parser_result);// yylval.val;
	x64_asm_init(&asm_ctx);

	x64_visit_ast(&asm_ctx, ast);
	printf("done");
	return res;
}