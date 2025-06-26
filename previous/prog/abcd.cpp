#include <unistd.h>
#include <iostream>
using namespace std;

struct len{
    int k[100];
};

int func(struct len tmp)
{
    asm("mov $10, %rax");
    int k = 0;
    return k;
}
void func(const char* k, ...){
    return;
}
int main()
{
    struct len tmp;
    tmp.k[10] = 1;
    cout << func(tmp);
    const char k = '1';
    func(&k);
}