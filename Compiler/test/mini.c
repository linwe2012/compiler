int gcd(int a, int b) {
    if (a % b == 0)
        return b;
    return gcd(b, a % b);
}

void disp_num(int n) {
	if (n < 0) {
		n = -n;
		putchar('-');
	}
	while (n) {
		putchar(n % 10 + '0');
		n = n / 10;
	}
}

int main(void) {
    disp_num(gcd(3 * 12, 8));
	putchar(10);
    return 0;
}