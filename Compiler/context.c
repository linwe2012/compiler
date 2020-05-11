void ctx_string_array_add(char*** p, int* n, char* new_str)
{
	char** new_p = (char**)malloc(sizeof(char*) * (*n + 1));
	memcpy(new_p, *p, sizeof(char*) * (*n));
	free(*p);

	new_p[*n + 1] = new_str;
	*p = new_p;
}