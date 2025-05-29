#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8081

using namespace std;

int main()
{
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address;
    cout << "Socket opened: " << socketfd << endl; 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(socketfd, (sockaddr *)&address, sizeof(address));
    cout << "bound to : " << PORT << endl;

    while (1)
    {
        int k = sizeof(address);
        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);

        ssize_t n = recvfrom(socketfd, buffer, sizeof(buffer) - 1, 0, (sockaddr *)&client_addr, &len);

        if (n < 0)
        {
            perror("recvfrom failed");
            close(socketfd);
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
            if(c == 0) ans = -1;
            else ans = a / c;
            break;
        case '*':
            ans = a * c;
            break;
        }
        char buf[10];
        sprintf(buf, "%d", ans);
        sendto(socketfd, (const char *)buf, sizeof(buf), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
        cout << "return val: " << ans << endl;
    }
}