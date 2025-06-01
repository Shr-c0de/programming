#include <iostream>

using namespace std;

class c{
    int a;
    void f(){
        a;
    }
    void func(){
        a++;
    }
};
struct d{
    int a;
};

int main(){
    cout << sizeof(c) << " " << sizeof(d) << endl;
}