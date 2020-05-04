#include <stdio.h>
extern void yyerror(const char*);  /* prints grammar violation message */
extern int yylex(void);
extern FILE* yyin;
extern FILE* yyout;
void parser_set_debug(int i);