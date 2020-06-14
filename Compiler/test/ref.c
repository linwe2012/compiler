struct A {
  int a;
  char b;
  double c;
  struct D{
    int k;
    struct E{
      int k;
    }e;
  }d;
  union U
  {
    int c;
    char b;
    double s;
    char x[10];
  }u;
  
};

//#include <stdio.h>
void putchar(int c);

void dummy(int a, float b);

struct Ch{
  int a;
};

void dummy2(int c, struct Ch, double e);

int main()
{
    struct Ch ch= { .a = 10 }; 
    struct A A_ins;
    A_ins.d.e.k = 10;
    int u = A_ins.d.e.k;
    int x = -10;
    int ck = x | 2;
    putchar(104); // 'h'
    putchar(111); // 'o'
    putchar(108); // 'l'
    putchar(97);  // 'a'
    putchar(10);  // '\n'
    dummy(1, 0.1);
    dummy2(8, ch, 0.8);
    int a = 10;
    if(ck == 1 && ck == 2){
      ++a;
    }
    if(ck==1 || ck==2){
      ++a;
    }
	int c[10][8] = {{ 2, a }};
    return 0;
}