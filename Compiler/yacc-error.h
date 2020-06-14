#pragma once
#include <stdio.h>
extern int debug;

extern int yylex(void);
extern int yyparse(void);
extern void yyerror(const char*);

extern void DumpRow(void);
extern int GetNextChar(char* b, int maxBuffer);
extern void BeginToken(char*);
extern void PrintError(char* s, ...);
void SetFile(FILE* _file);

struct FileLocation
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	char* current_line;
};

extern struct FileLocation fileloc;
extern int error_encountered;