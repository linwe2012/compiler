#include <string.h>
#include <stdlib.h>
int str_equal(const char* a, const char* b)
{
	return strcmp(a, b) == 0;
}

char* str_concat(const char* a, const char* b)
{

	int la = strlen(a);
	int lb = strlen(b);
	int sz = la + lb + 1;

	char* res = malloc(sz);
	memcpy(res, a, la);
	memcpy(res+la, b, lb+1);
	
	return res;
}