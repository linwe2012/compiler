void putchar(int c);

/*

int dummy(int a, float b, double c);

int main(int argc, char** argv)
{
    putchar(104); // 'h'
    putchar(111); // 'o'
    putchar(108); // 'l'
    putchar(97);  // 'a'
    putchar(10);  // '\n'
    int x = dummy(8, 0.8f, (12 + 4 % 3) * 5.5);
    return 0;
}

*/

int main(int argc, char* argv[]) {
    int x = 10;
    if ((x = (++x * 88 << 2) % 7)) {
        if (!x) {
            return 2;
        }
        return x;
    } else if (x < 10) {
        return 1;
    } else {
        putchar(--x);
        return --x;
    }
    return 0;
}