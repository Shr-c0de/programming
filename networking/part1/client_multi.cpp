#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#define SERVER_PORT 12345
#define CHUNK 4096

struct thread_args {
    char ip[64];
    char topic[128];
};

void *fetch_topic_thread(void *arg);

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
        fprintf(stderr, "[%s] Connection dropped or error.\n", subject);
        return;
    }

    buf[nbytes] = '\0';
    if (strncmp(buf, "ERROR", 5) == 0)
    {
        fprintf(stderr, "[%s] %s", subject, buf);
        return;
    }

    int file_count = atoi(buf);
    printf("[%s] Files to download: %d\n", subject, file_count);

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
                fprintf(stderr, "[%s] Unexpected connection close.\n", subject);
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
                fprintf(stderr, "[%s] Header overflow.\n", subject);
                return;
            }
        }

        char fname[256];
        long fsize;
        if (sscanf(buf, "%s %ld", fname, &fsize) != 2)
        {
            fprintf(stderr, "[%s] Malformed file header.\n", subject);
            return;
        }

        printf("[%s] Downloading: %s (%ld bytes)\n", subject, fname, fsize);

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
                fprintf(stderr, "[%s] Transfer interrupted.\n", subject);
                close(out_fd);
                return;
            }
            write(out_fd, buf, nbytes);
            got += nbytes;
        }
        close(out_fd);
    }

    printf("[%s] All files received successfully.\n", subject);
}

void *fetch_topic_thread(void *arg)
{
    struct thread_args *targs = (struct thread_args *)arg;
    const char *ip_addr = targs->ip;
    const char *subject = targs->topic;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        pthread_exit(NULL);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, ip_addr, &server.sin_addr) <= 0)
    {
        fprintf(stderr, "[%s] Invalid or unsupported address.\n", subject);
        close(sockfd);
        pthread_exit(NULL);
    }

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        close(sockfd);
        pthread_exit(NULL);
    }

    fetch_topic_files(sockfd, subject);
    close(sockfd);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *ip_addr = argv[1];
    const char *topics[] = {"Cane", "Pecora", "Gatto"};
    pthread_t threads[3];
    struct thread_args args[3];

    for (int i = 0; i < 3; i++)
    {
        strncpy(args[i].ip, ip_addr, sizeof(args[i].ip) - 1);
        strncpy(args[i].topic, topics[i], sizeof(args[i].topic) - 1);
        args[i].ip[sizeof(args[i].ip) - 1] = '\0';
        args[i].topic[sizeof(args[i].topic) - 1] = '\0';

        if (pthread_create(&threads[i], NULL, fetch_topic_thread, &args[i]) != 0)
        {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < 3; i++)
        pthread_join(threads[i], NULL);

    printf("All threads finished.\n");
    return EXIT_SUCCESS;
}
