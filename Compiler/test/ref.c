#include <stdio.h>
// void putchar(int c);

void dummy(int a, float b);

struct Ch{
  int a;
};

void dummy2(int c, struct Ch, double e);

int main()
{
    struct Ch ch= { .a = 10 }; 
    putchar(104); // 'h'
    putchar(111); // 'o'
    putchar(108); // 'l'
    putchar(97);  // 'a'
    putchar(10);  // '\n'
    dummy(1, 0.1);
    dummy2(8, ch, 0.8);
    return 0;
}