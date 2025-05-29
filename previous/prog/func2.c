#include<stdio.h>

int a = 10;
int d;
void f(int ** p){
	static int a = 20;
	*p = &a;
}
int main(){
	int a = 10, *p = &a;
	f(&p);
	printf("%d, %d, %d", *p);
}
