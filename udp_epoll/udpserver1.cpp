#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

#define PORT0 8081
#define PORT1 8082

using namespace std;

void 
func(int sockfd)
{
    static char buffer[20];
    memset(&buffer, 0, 20);
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (sockaddr *)&client_addr, &len);

    if (n < 0)
    {
        perror("recvfrom failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0';
    cout << "RECV : " << buffer << endl;
    int a, c, ans;
    char b;
    sscanf(buffer, "%d %c %d", &a, &b, &c);
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
    char buf[10];
    sprintf(buf, "%d", ans);
    sendto(sockfd, (const char *)buf, sizeof(buf), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
    cout << "return val: " << ans << endl;
}

int main()
{
    // sockets ----------------------------------------

    int sock0 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock0 <= 0)
    {
        cerr << "sock0 failed\n";
        exit(0);
    }
    cout << "Socket opened: " << sock0 << endl;

    int sock1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock1 <= 0)
    {
        cerr << "sock1 failed\n";
        exit(0);
    }
    cout << "Socket opened: " << sock1 << endl;

    // bind addresses -----------------------------------------

    struct sockaddr_in server0, server1;
    memset(&server0, 0, sizeof(server0));
    memset(&server1, 0, sizeof(server1));

    server0.sin_family = AF_INET;
    server0.sin_addr.s_addr = INADDR_ANY;
    server0.sin_port = htons(PORT0);
    bind(sock0, (sockaddr *)&server0, sizeof(server0));
    cout << "bound to : " << PORT1 << endl;

    server1.sin_family = AF_INET;
    server1.sin_addr.s_addr = INADDR_ANY;
    server1.sin_port = htons(PORT0);
    bind(sock1, (sockaddr *)&server1, sizeof(server1));
    cout << "bound to : " << PORT1 << endl;

    // epoll ---------------------------------------------------------
    struct epoll_event eve, events[10];
    int epollfd;

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    eve.events = EPOLLIN;
    eve.data.fd = sock0;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock0, &eve) == -1)
    {
        perror("epoll_ctl: sock 0");
        exit(EXIT_FAILURE);
    }
    eve.events = EPOLLIN;
    eve.data.fd = sock1;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock1, &eve) == -1)
    {
        perror("epoll_ctl: sock 1");
        exit(EXIT_FAILURE);
    }

    // start -------------------------------------------------------
    while (1)
    {
        int event_count = epoll_wait(epollfd, events, 10, 30000);
        printf("%d ready events\n", event_count);
        for (int i = 0; i < event_count; i++)
        {
            func(events[i].data.fd);
        }
    }
}