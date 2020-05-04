void putchar(int c);

void dummy(int a, float b, double c);


int main()
{
    putchar(104); // 'h'
    putchar(111); // 'o'
    putchar(108); // 'l'
    putchar(97);  // 'a'
    putchar(10);  // '\n'
    dummy(8, 0.8f, 0.9);
    return 0;
}