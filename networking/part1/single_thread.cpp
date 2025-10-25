#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define LISTEN_PORT 12345
#define BUF_SIZE 4096

static int srv_fd = -1; // Global for signal handler

// Static mapping: topic → folder
struct TopicMap {
    const char *key;
    const char *path;
} topic_map[] = {
    {"Cane", "/home/shreyas/programming/networking/archive/raw-img/cane"},
    {"Cavallo", "/home/shreyas/programming/networking/archive/raw-img/cavallo"},
    {"Elefante", "/home/shreyas/programming/networking/archive/raw-img/elefante"},
    {"Farfalla", "/home/shreyas/programming/networking/archive/raw-img/farfalla"},
    {"Gallina", "/home/shreyas/programming/networking/archive/raw-img/gallina"},
    {"Gatto", "/home/shreyas/programming/networking/archive/raw-img/gatto"},
    {"Mucca", "/home/shreyas/programming/networking/archive/raw-img/mucca"},
    {"Pecora", "/home/shreyas/programming/networking/archive/raw-img/pecora"},
    {"Ragno", "/home/shreyas/programming/networking/archive/raw-img/ragno"},
    {"Scoiattolo", "/home/shreyas/programming/networking/archive/raw-img/scoiattolo"},
    {NULL, NULL}
};
void handle_sigint(int sig) {
    (void)sig; // Unused
    printf("\n[!] Caught Ctrl+C, shutting down server...\n");
    if (srv_fd != -1) {
        close(srv_fd);
        printf("[+] Closed listening socket.\n");
    }
    exit(EXIT_SUCCESS);
}

// Look up folder path for a topic
static const char *lookup_path(const char *topic) {
    for (int i = 0; topic_map[i].key; i++) {
        if (strcmp(topic_map[i].key, topic) == 0)
            return topic_map[i].path;
    }
    return NULL;
}

// Send one file (header + data)
static void transmit_file(int client, const char *fullpath) {
    int fd = open(fullpath, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return;
    }

    const char *fname = strrchr(fullpath, '/');
    fname = fname ? fname + 1 : fullpath;

    char header[512];
    snprintf(header, sizeof(header), "%s %ld\n", fname, st.st_size);
    send(client, header, strlen(header), 0);

    char buf[BUF_SIZE];
    ssize_t r;
    while ((r = read(fd, buf, sizeof(buf))) > 0)
        send(client, buf, r, 0);

    close(fd);
}

// Send all files for a given topic
static void handle_topic(int client, const char *topic) {
    const char *dirpath = lookup_path(topic);
    if (!dirpath) {
        const char *msg = "ERROR Unknown Topic\n";
        send(client, msg, strlen(msg), 0);
        return;
    }

    DIR *dp = opendir(dirpath);
    if (!dp) {
        const char *msg = "ERROR Cannot Access Directory\n";
        send(client, msg, strlen(msg), 0);
        return;
    }

    // Count regular files
    int count = 0;
    struct dirent *de;
    while ((de = readdir(dp)) != NULL)
        if (de->d_type == DT_REG)
            count++;

    rewinddir(dp);

    char count_msg[64];
    snprintf(count_msg, sizeof(count_msg), "%d\n", count);
    send(client, count_msg, strlen(count_msg), 0);

    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_REG) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dirpath, de->d_name);
            transmit_file(client, path);
        }
    }

    closedir(dp);
}

int main() {
    // Register SIGINT handler
    signal(SIGINT, handle_sigint);

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(LISTEN_PORT);

    if (bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("bind");
        close(srv_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(srv_fd, 5) < 0) {
        perror("listen");
        close(srv_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server ready on port %d (Ctrl+C to stop)\n", LISTEN_PORT);

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int cli_fd = accept(srv_fd, (struct sockaddr *)&cli_addr, &cli_len);
        if (cli_fd < 0) {
            perror("accept");
            continue;
        }

        printf("Connection from %s\n", inet_ntoa(cli_addr.sin_addr));

        char buf[BUF_SIZE];
        ssize_t n = recv(cli_fd, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';
            if (strncmp(buf, "GET ", 4) == 0) {
                char *req_topic = buf + 4;
                char *nl = strchr(req_topic, '\n');
                if (nl) *nl = '\0';
                printf("Requested topic: %s\n", req_topic);
                handle_topic(cli_fd, req_topic);
            } else {
                const char *msg = "ERROR Bad Request\n";
                send(cli_fd, msg, strlen(msg), 0);
            }
        }

        close(cli_fd);
    }

    close(srv_fd);
    return 0;
}