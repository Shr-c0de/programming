#include <netinet/in.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main(){
    const char* addr = "tmp.io";
    mkfifo(addr, 0666);
    int fd = open(addr, O_APPEND);
    write(fd, "hello pipes", 12);
    close(fd);
    fd = open(addr, O_RDONLY);
    char buf[15];
    read(fd, buf, 15);
    cout << buf << endl;

}               