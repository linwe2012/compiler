// char *xa[5];
// char *( *(*xvar)() )[10]; <- 可以解析但是没法LLVM 编译
// Windows API
//int __stdcall SetConsoleTextAttribute(void * hConsoleOutput, unsigned short wAttributes);
//__stdcall void* GetStdHandle(unsigned int nStdHandle); 

struct Demo {
	int a;
	void* b;
	char c;
};

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
	struct Demo demo_b;
	struct A{
	int A_a;
	double A_b;
	} st_A;
	int nb = sizeof(struct A);

    disp_num(gcd(3 * 12, 60));
	putchar(10);

	printf("\nHello world");
	printf("My age is %d\n, Im %f m, and my hobby is %d\n", 22, 1.8, "Running");
    return 0;
}

int g_a = 10;

void test(void)
{
	g_a = 100;
	int a[10];
	int arr[10][2];
	a[1] = arr[2][1];
	arr[3][1] = a[2];
	double d[3][10];
	int na = sizeof(a);
	int nd = sizeof(d);
}