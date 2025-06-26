#include <stdio.h>
#include <unistd.h>

int main()
{
	int i = 1;
	for (int i = 0; i < 20; i++)
		fork();
	while (i++)
	{
		printf("count = %d\n", i);
		float k = 12394621798 / 123797823;
		int t = k*k+23;
		printf("count = %d, %d", i, t);
		//		gettimeofday();
	}
}
