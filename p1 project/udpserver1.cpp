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


#define PORT0 8081
#define PORT1 8082

#define processcount 10

int childpid[processcount];

extern void* init_process();

int main(){
    void* vector = init_process();
}