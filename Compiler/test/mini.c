// char *xa[5];
// char *( *(*xvar)() )[10]; <- 可以解析但是没法LLVM 编译



int printf(char const* format, ...);

int gcd(int a, int b) {
    if (a % b == 0)
        return b;
    return gcd(b, a % b);
}

void disp_num(int n) {
	if (n < 0) {
		n = -n;
		putchar('-');
	} else if (n == 0) {
		return;
	}
	
	disp_num(n / 10);
	putchar(n % 10 + '0');
}

int main(void) {
	struct A{
	int A_a;
	double A_b;
	} st_A;

    disp_num(gcd(3 * 12, 60));
	putchar(10);

	printf("\nHello world");
	printf("My age is %d\n, Im %f m, and my hobby is %d\n", 22, 1.8, "Running");
    return 0;
}
void test(void)
{
	int a[10];
	a[1] = 10;
}