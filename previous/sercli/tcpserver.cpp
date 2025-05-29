#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#define MAX 80
#define PORT0 8081 // TCP
#define PORT1 8082 // TCP

typedef struct sockaddr SA;
using namespace std;

void func(int connfd)
{
    char buff[MAX];
    memset(buff, 0, sizeof(buff));
    read(connfd, buff, sizeof(buff));

    int a, c;
    float ans;
    char b;

    sscanf(buff, "%d %c %d", &a, &b, &c);
    cout << a << " " << b << " " << c << " = ";

    switch (b)
    {
    case '+':
        ans = a + c;
        break;
    case '-':
        ans = a - c;
        break;
    case '*':
        ans = a * c;
        break;
    case '/':
        ans = (c == 0) ? -1 : (float)a / c;
        break;
    default:
        ans = -1;
    }

    char result[20];
    sprintf(result, "%.2f", ans);
    cout << result << endl;

    write(connfd, result, strlen(result) + 1);

    // close(connfd);
    return;
}

int main()
{
    int sockfd0, sockfd1, connfd, len;
    struct sockaddr_in servaddr0, servaddr1, cli;
    len = sizeof(cli);

    //   sockets  ---------------------------------------
    sockfd0 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd0 == -1)
    {
        printf("socket 0 creation failed...\n");
        exit(0);
    }
    else
        printf("Socket 0 successfully created..\n");

    sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd1 == -1)
    {
        printf("socket 1 creation failed...\n");
        exit(0);
    }
    else
        printf("Socket 1 successfully created..\n");

    //  addresses  -------------------------------------
    memset(&servaddr0, 0, sizeof(servaddr1));
    memset(&servaddr1, 0, sizeof(servaddr1));

    servaddr0.sin_family = AF_INET;
    servaddr0.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr0.sin_port = htons(PORT0);

    servaddr1.sin_family = AF_INET;
    servaddr1.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr1.sin_port = htons(PORT1);

    //  bind  -----------------------------------------
    if ((bind(sockfd0, (SA *)&servaddr0, sizeof(servaddr0))) != 0)
    {
        printf("socket 0 bind failed...\n");
        exit(0);
    }
    else
        printf("Socket 0 successfully bound..\n");

    if ((bind(sockfd1, (SA *)&servaddr1, sizeof(servaddr1))) != 0)
    {
        printf("socket 1 bind failed...\n");
        exit(0);
    }
    else
        printf("Socket 1 successfully bound..\n");

    // listen -----------------------------------------

    if ((listen(sockfd0, 5)) != 0)
    {
        printf("Listen 0 failed...\n");
        exit(0);
    }
    else
        printf("Server 0 listening..\n");

    if ((listen(sockfd1, 5)) != 0)
    {
        printf("Listen 1 failed...\n");
        exit(0);
    }
    else
        printf("Server 1 listening..\n");

    // setup select --------------------------------------------

    cout << "Server is up in: " << PORT0 << " " << PORT1 << endl;
    int s0 = -1, s1 = -1, sockfd;
    int map = 0;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    while (true)
    {
        // cout << "checkpoint1" << endl;
        if (map ^ 0b11)
        {

            fd_set set;
            FD_ZERO(&set);
            if (map ^ 0x1)
                FD_SET(sockfd0, &set);
            if (map ^ 0xa)
                FD_SET(sockfd1, &set);
            if (select((sockfd0 > sockfd1 ? sockfd0 : sockfd1) + 1, &set, NULL, NULL, &timeout) > 0)
            {
                if (FD_ISSET(sockfd0, &set) > 0)
                {
                    s0 = accept(sockfd0, (SA *)&cli, (socklen_t *)&len);
                    map |= 0b1;
                }
                else if (FD_ISSET(sockfd1, &set) > 0)
                {
                    s1 = accept(sockfd1, (SA *)&cli, (socklen_t *)&len);
                    map |= 0b10;
                }
            }
        }
        // cout << "checkpoint2" << endl;
        fd_set set2;
        FD_ZERO(&set2);
        if (map & 0x1)
            FD_SET(s0, &set2);
        if (map & 0xa)
            FD_SET(s1, &set2);

        if (select((s0 > s1 ? s0 : s1) + 1, &set2, NULL, NULL, &timeout) > 0)
        {

            if (map & 0b1 && FD_ISSET(s0, &set2))
            {
                sockfd = s0;
            }
            else if (map & 0b10 && FD_ISSET(s1, &set2))
            {
                sockfd = s1;
            }
            else
                continue;
        }
        else sockfd = -1;
        // cout << "checkpoint3" << endl;
        if (sockfd < 0)
        {
            continue;
        }
        func(sockfd);
        // cout << "checkpoint4" << endl;
    }
}