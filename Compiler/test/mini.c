void putc(int x);

int foo(int x, int y) {
    for (int i = 0; i < x; ++i) {
        putc(i + y);
    }
    return x + y;
}