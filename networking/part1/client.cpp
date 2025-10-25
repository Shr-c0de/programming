#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SERVER_PORT 12345
#define CHUNK 4096

void fetch_topic_files(int client_fd, const char *subject)
{
    char buf[CHUNK];
    ssize_t nbytes;

    char query[256];
    snprintf(query, sizeof(query), "GET %s\n", subject);
    send(client_fd, query, strlen(query), 0);

    nbytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (nbytes <= 0)
    {
        fprintf(stderr, "Connection dropped or error.\n");
        return;
    }

    buf[nbytes] = '\0';
    if (strncmp(buf, "ERROR", 5) == 0)
    {
        fprintf(stderr, "%s", buf);
        return;
    }

    int file_count = atoi(buf);
    printf("Files to download: %d\n", file_count);

    struct stat info = {0};
    if (stat(subject, &info) == -1)
        mkdir(subject, 0700);

    for (int f = 0; f < file_count; f++)
    {
        int pos = 0;
        while (1)
        {
            nbytes = recv(client_fd, buf + pos, 1, 0);
            if (nbytes <= 0)
            {
                fprintf(stderr, "Unexpected connection close.\n");
                return;
            }
            if (buf[pos] == '\n')
            {
                buf[pos] = '\0';
                break;
            }
            pos++;
            if (pos >= CHUNK - 1)
            {
                fprintf(stderr, "Header overflow.\n");
                return;
            }
        }

        char fname[256];
        long fsize;
        if (sscanf(buf, "%s %ld", fname, &fsize) != 2)
        {
            fprintf(stderr, "Malformed file header.\n");
            return;
        }

        printf("Downloading: %s (%ld bytes)\n", fname, fsize);

        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", subject, fname);
        int out_fd = open(fullpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd < 0)
        {
            perror("open");
            return;
        }

        long got = 0;
        while (got < fsize)
        {
            int want = CHUNK;
            if (fsize - got < want)
                want = fsize - got;

            nbytes = recv(client_fd, buf, want, 0);
            if (nbytes <= 0)
            {
                fprintf(stderr, "Transfer interrupted.\n");
                close(out_fd);
                return;
            }
            write(out_fd, buf, nbytes);
            got += nbytes;
        }
        close(out_fd);
    }

    printf("All files received successfully.\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <topic>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *ip_addr = argv[1];
    const char *subject = argv[2];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, ip_addr, &server.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid or unsupported address.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    fetch_topic_files(sockfd, subject);
    close(sockfd);

    return EXIT_SUCCESS;
}
