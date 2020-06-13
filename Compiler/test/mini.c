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
    disp_num(gcd(3 * 12, 60));
	putchar(10);
    return 0;
}