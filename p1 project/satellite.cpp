#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <signal.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT0 8081
#define PORT1 8082

using namespace std;

void func(char *buffer)
{
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cerr << "atleast 4 argument requiured\n";
        for (int i = 0; i < argc; i++)
            cout << argv[i] << endl;
        return 0;
    }
    int readfd = atoi(argv[1]), writefd = atoi(argv[2]);
    cout << "satellite " << argv[3] << " reporting for duty" << endl;
    cout << "reading from pipe: " << readfd << " writing to pipe: " << writefd << endl;  
    // epoll -------------------------------------------------------

    struct epoll_event eve, events[10];
    int epollfd;

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    eve.events = EPOLLIN;
    eve.data.fd = readfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, readfd, &eve) == -1)
    {
        printf("readfd in satellite %s", argv[3]);
        exit(EXIT_FAILURE);
    }

    // start -------------------------------------------------------
    while (1)
    {
        int len = epoll_wait(epollfd, events, 10, 10000);
        if (len)
        {
            char buf[10];
            read(events[0].data.fd, buf, 10);
            int a, c, ans;
            char b;
            sscanf((const char *)buf, "%d %c %d", &a, &b, &c);
            switch (b)
            {
            case '+':
                ans = a + c;
                break;
            case '-':
                ans = a - c;
                break;
            case '/':
                if (c == 0)
                    ans = -1;
                else
                    ans = a / c;
                break;
            case '*':
                ans = a * c;
                break;
            }
            sprintf(buf, "%d %s", ans, argv[3]);
            write(writefd, buf, 10);
        }
    }
}