int foo(int x);

int bar(int a, int b, int c);

int foo(int x) {
    int a = 10;
    while (1) {
        if (1) {
            return bar(1, 2, 3);
        } else {
            return 3;
        }
    }
    return 0;
}