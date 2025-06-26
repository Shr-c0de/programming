#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

int main()
{

    char *ptr[1024];
    // ptr[0] = (char *)malloc(1000);
    // cout << ptr[0][1] << endl;
    asm("jmp 1x ");
    for (int i = 0; i < 1024; i++)
    {
        ptr[i] = (char *)malloc(1024 * 1024);
        printf("%x\n", ptr[i]);
        //usleep(1000);
    }
     cout << "allocated data" << endl;

    sleep(60);
    cout << "freeing data" << endl;

    for (int i = 0; i < 512; i++)
    {
        free(ptr[i*2]);

    }
     cout << "freeed data" << endl;
    sleep(60);
    for (int i = 0; i < 512; i++)
    {
        ptr[i] = (char *)malloc(1024 * 1024);
        printf("%x\n", ptr[i]);
        usleep(1000);
    }
}