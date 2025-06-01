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

using namespace std;

int *pipesend[2], *piperead[2];
map<int, int> pipe_map;

void *init_process(int processcount, int *childpid, int ***readfd)
{
    void *function_vector[3]; // 0->init, 1->read, 2-write;
    function_vector[0] = &setup_process_pipe;
    function_vector[1] = &pipe_read;
    function_vector[2] = &pipe_write;
    return function_vector;
    pipesend[0] = (int *)calloc(processcount, sizeof(int));
    pipesend[1] = (int *)calloc(processcount, sizeof(int));
    piperead[0] = (int *)calloc(processcount, sizeof(int));
    piperead[1] = (int *)calloc(processcount, sizeof(int));
    *readfd = piperead;
    setup_process_pipe(processcount, childpid);
}

int pipe_read(char *buf, int size, int pipefd)
{
    return read(pipefd, buf, size);
}

int pipe_write(char *buf, int size, int pipefd){
    write(pipe_map[pipefd], buf, size);
}

int setup_process_pipe(int processcount, int *childpid)
{
    if (processcount < 1)
    {
        cerr << "Minimum 1 helper process needed\n";
        return 0;
    }

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

        pipe_map[piperead[i][0]] = piperead[i][1]; // read pipe
        pipe_map[piperead[i][1]] = piperead[i][0];
        pipe_map[pipesend[i][0]] = pipesend[i][1];
        pipe_map[pipesend[i][1]] = pipesend[i][0];

        char arg1[3], arg2[3], arg3[5];
        sprintf(arg1, "%d", pipesend[i][0]);
        sprintf(arg2, "%d", piperead[i][1]);
        sprintf(arg3, "%d", i);
        int k;
        if ((k = fork()) == 0)
        {
            execlp("./satellite", "./satellite", arg1, arg2, arg3, NULL);
        }
        childpid[i] = k;
    }
    cout << "Pipe interface up \n";
}