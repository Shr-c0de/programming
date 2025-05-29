#include <cstddef>
#include<stdio.h>
struct a1{
	int a;
	char b;
	short c;
};
struct a2{
	int a;
	short b;
	char c;
};
struct a3{
	short a;
	int b;
	char c;
};
struct a4{
	short a;
	char b;
	int c;
};
struct a5{
	char a;
	int b;
	short c;
};
struct a6{
	char a;
	short b;
	int c;
};
union u1{
	a1 a;
	a2 b;
	a3 c;
	a4 d;
	a5 e;
	a6 f;
};

struct all{
	a1 a;
	a2 b;
	a3 c;
	a4 d;
	a5 e;
	a6 f;
};
void fun(char *c){
	int k = offsetof(a3, c);
	a3* tmp =(a3*)((char*)c-k);
	printf("\n\n%hd, %d, %c\n\n", tmp->a, tmp->b, tmp->c);
}
int main(){
	//printf("%d %d %d %d %d %d\n", sizeof(a1), sizeof(a2), sizeof(a3), sizeof(a4), sizeof(a5), sizeof(a6));
	//printf("8, 8, 12, 8, 12, 8\n"); 
	//printf("%d, %d\n", sizeof(u1), offsetof(all, c));
	//printf("12, 60\n");
	
	a3 num1;
	num1.a = 2;
	num1.b = 1000;
	num1.c = 'c';

	fun(&num1.c);
	
	
	
	
	return 1;
}
