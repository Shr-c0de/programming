#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> 
#include <random>
using namespace std;

#define MAX 80
#define SA struct sockaddr

const char oper[5] = "+-*/";

void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;) {
        usleep(rand()%1000000);
        // sleep_ms
        int a = rand() % 100, c = rand() % 100;
        char b = oper[rand() % 4];
        sprintf(buff, "%d %c %d", a, b, c);
        write(sockfd, buff, strlen(buff)+1);
        //bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        cout << a << " " << b << " " << c << " = " << buff <<endl;
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2) {
        cout << "atleast 1 argument required\n";
        exit(-1);
    }
    int PORT;
    PORT = atoi(argv[1]);
    
    cout << "selected port: " << PORT << endl;

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    srand((unsigned)time(NULL));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cout << "fail" << endl;
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.8");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
}