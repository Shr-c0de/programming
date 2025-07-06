#include <iostream>
using namespace std;

struct tmp{
    int a;
    int arr[10];
};

struct tmp func(int a, int b, int c, int d, int e, int f, int g, int h){
    struct tmp k;
    k.a = a+b+c+d+e+f+g+h;
    return k;
}

int main(){
    struct tmp a = func(1, 2, 3, 4, 5, 6, 7, 8);
    cout << a.a << endl;
}