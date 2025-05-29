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
#include <sys/wait.h>
#include <unistd.h>
#include <map>

#define PORT0 8081
#define PORT1 8082

#define processcount 10

using namespace std;

class assigned_process
{
public:
    int fd;
    sockaddr_in add;
    assigned_process()
    {
        fd = -1;
        add;
    }
    assigned_process(int a, sockaddr_in sock)
    {
        fd = a;
        add = sock;
    }
};

int childpid[processcount];

void handler(int sfd, int  sock0, int sock1)
{
    struct signalfd_siginfo fdsi;
    ssize_t s = read(sfd, &fdsi, sizeof(fdsi));

    if (s != sizeof(fdsi))
    {
        perror("read signalfd");
        return;
    }
    if (fdsi.ssi_signo == SIGINT)
    {
        cout << "Caught SIGINT\n";
        cout << "child process list: ";
        for (int i = 0; i < processcount; i++)
        {
            cout << childpid[i] << " ";
            kill(childpid[i], 9);
            waitpid(childpid[i], NULL, 0);
        }
        close(sock0);
        close(sock1);
        cout << endl;
        exit(0);
    }
    else if (fdsi.ssi_signo == SIGQUIT)
    {
        cout << "Caught SIGQUIT\n";
    }
}

int main()
{
    // processes ----------------------------------------------------

    if (processcount < 1)
    {
        cerr << "Minimum 1 helper process needed\n";
        return 0;
    }

    int pipesend[processcount][2], piperead[processcount][2];
    // 0 -> read. 1->write
    cout << "Server started\n creating " << processcount << " processes\n";

    for (int i = 0; i < processcount; i++)
    {
        // create pipes
        if (pipe(pipesend[i]) < 0)
        {
            cerr << "pipesend not working\n";
            return 0;
        }
        if (pipe(piperead[i]) < 0)
        {
            cerr << "piperead not working\n";
            return 0;
        }
        cout << "Pipes send: " << pipesend[i][0] << " " << pipesend[i][1] << " | read: " << piperead[i][0] << " " << piperead[i][1] << endl;

        // forking

        char arg1[3], arg2[3], arg3[5];
        sprintf(arg1, "%d", pipesend[i][0]);
        sprintf(arg2, "%d", piperead[i][1]);
        sprintf(arg3, "%d", i);
        int k;
        if ((k = fork()) == 0)
        {
            close(pipesend[i][1]);
            close(piperead[i][0]);
            execlp("./satellite", "./satellite", arg1, arg2, arg3, NULL);
        }
        childpid[i] = k;

        // adding to epoll

        close(pipesend[i][0]);
        close(piperead[i][1]);
        // this process has: pipesend[i][1] and piperead[i][0];
    }
    sleep(1);
    cout << "Pipe interface up \n";



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
    cout << "bound to : " << PORT0 << endl;

    server1.sin_family = AF_INET;
    server1.sin_addr.s_addr = INADDR_ANY;
    server1.sin_port = htons(PORT1);
    bind(sock1, (sockaddr *)&server1, sizeof(server1));
    cout << "bound to : " << PORT1 << endl;

    // signal --------------------------------------------------------

    int sfd;
    ssize_t s;
    sigset_t mask;
    struct signalfd_siginfo fdsi;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
    {
        cerr << "error at sigprocmask\n";
        return 2048;
    }
    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
        err(EXIT_FAILURE, "signalfd");

    // epoll ---------------------------------------------------------
    struct epoll_event eve, events[30];
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

    eve.events = EPOLLIN;
    eve.data.fd = sfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sfd, &eve) == -1)
    {
        perror("epoll_ctl: sfd");
        exit(EXIT_FAILURE);
    }
    cout << "sock0, sock1, cfd added\n";

    for (int i = 0; i < processcount; i++)
    {
        eve.events = EPOLLIN;
        eve.data.fd = piperead[i][0];
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, piperead[i][0], &eve) == -1)
        {
            printf("epoll_ctl: pipe read %d", piperead[i][0]);
            exit(EXIT_FAILURE);
        }
    }

    // start -------------------------------------------------------
    int proc = 0;
    char arg[6];

    map<int, assigned_process> mp;

    while (1)
    {
        int event_count = epoll_wait(epollfd, events, 10, 10000);

        for (int i = 0; i < event_count; i++)
        {
            printf("%d ready events\n", event_count);
            int is_process = 0;
            for (int m = 0; m < processcount; m++)
            {
                if (events[i].data.fd == piperead[m][0])
                {
                    is_process = 1;
                    break;
                }
            }

            if (events[i].data.fd == sfd) // signal
            {
                handler(sfd, sock0, sock1);
                continue;
            }
            else if (is_process) // pipe
            {
                //cout << "process detected\n";
                char buf[10];
                read(events[i].data.fd, buf, 10);
                //cout << buf << endl;
                int ret_process_id, ans;
                sscanf(buf, "%d %d", &ans, &ret_process_id);
                assigned_process tmp = mp[ret_process_id];

                sendto(tmp.fd, (const char *)buf, sizeof(buf), MSG_CONFIRM, (const struct sockaddr *)&tmp.add, sizeof(sockaddr_in));
            }
            else // socket
            {
                // socket
                //cout << "socket detected\n";
                static char buffer[20];
                memset(&buffer, 0, 20);
                sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);

                ssize_t n = recvfrom(events[i].data.fd, buffer, sizeof(buffer) - 1, 0, (sockaddr *)&client_addr, &len);

                if (n < 0)
                {
                    perror("recvfrom failed");
                    close(events[i].data.fd);
                    exit(EXIT_FAILURE);
                }

                buffer[n] = '\0';

                sprintf(arg, "%d", events[i].data.fd);
                write(pipesend[proc][1], buffer, n);
                cout << "sent to: " << proc << endl;
                mp[proc] = assigned_process(events[i].data.fd, client_addr);
                if (proc < processcount - 1)
                    proc++;
                else
                    proc = 0;
            }
        }
    }
}