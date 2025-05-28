#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <random>

#define SERVERPORT 8081

using namespace std;

const char oper[5] = "+-*/";

int main()
{
    srand((unsigned)time(NULL));

    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    cout << "socket opened : " << socketfd << endl;
    
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVERPORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    int n;
    socklen_t len;
    char send_data[100], rec[10];
    int tmp;
    while (1)
    {
        sprintf(send_data, "%d %c %d", rand() % 100, oper[rand() % 4], rand() % 100);
        cout << send_data << " = ";
        sendto(socketfd, (const char *)send_data, sizeof(send_data), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        ssize_t n = recvfrom(socketfd, rec, sizeof(rec) - 1, 0, (sockaddr *)&servaddr, &len);

        if (n < 0)
        {
            perror("recvfrom failed");
            close(socketfd);
            exit(EXIT_FAILURE);
        }
        cout << rec << endl;
    }
}