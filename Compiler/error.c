#include "error.h"
#include <stdio.h>
#include <windows.h>   // WinApi header
#include "yacc-error.h"

void cc_log_error(const char* file, int line,  AST* ast, const char* message, ...)
{
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	if (ast == NULL)
	{
		printf("%s:%d: ", file, line);
	}
	va_list args;
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	printf("\n");

#ifdef THROW_EXCEPT_ON_LOG_INTERNAL_ERROR
	assert(0);
#endif // THROW_EXCEPT_ON_LOG_INTERNAL_ERROR
}

void yyerror(char const* s) {

	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	fprintf(stderr, "%s\n", s);
	

	printf("At line %d:\n", fileloc.first_line);
	printf("%s", fileloc.current_line);
	int i = 0;
	for ( ;i < fileloc.first_column; ++i)
	{
		putchar(' ');
	}

	putchar('^');
	++i;
	for (; i < fileloc.last_column; ++i)
	{
		putchar('~');
	}
	printf("\n");
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	error_encountered = 1;
}



