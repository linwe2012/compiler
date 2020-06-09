int foo(int x);

int bar(int a, int b, int c);

int foo(int x) {
	int a = 10;
	while (1) {
		if (1) {
			return bar(1, 2, 3);
		}
		else {
			return a + 3;
		}
	}
	return 0;
}

/*int foo(int a);

int main(void) {
	int h = 104, o = 111, l = 108, a = 97;
	putchar(h); // 'h'
	putchar(o); // 'o'
	putchar(l); // 'l'
	putchar(a);  // 'a'
	putchar(10);  // '\n'
	return 0;
}*/