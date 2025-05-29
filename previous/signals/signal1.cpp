#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
void singalHandler(int sig)
{
    printf("Caught signal %d\n", sig);
}

int main()
{
    signal(SIGINT, singalHandler);
    cout << getpid() << endl;
    while (1)
    {
        printf("Hello World!\n");
        sleep(1);
    }
    return 0;
}