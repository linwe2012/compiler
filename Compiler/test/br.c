// clang -cc1 br.c -emit-llvm -O0

int main()
{
    int i = 10;
    if(i == 1){
        return -1;
    }
    else if(i == 2)
    {
        return -2;
    }
    else {
        return 0;
    }
}