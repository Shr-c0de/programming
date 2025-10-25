#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define LISTEN_PORT 12345
#define BUF_SIZE 4096

static int srv_fd = -1;  // global server fd, closed on Ctrl+C

struct TopicMap {
    const char *topic;
    const char *folder;
} mappings[] = {
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

// graceful shutdown handler
static void handle_sigint(int sig) {
    (void)sig;
    if (srv_fd >= 0) {
        printf("\n[INFO] Closing server socket...\n");
        close(srv_fd);
    }
    printf("[INFO] Server exiting gracefully.\n");
    exit(0);
}

static const char *find_path(const char *t) {
    for (int i = 0; mappings[i].topic; i++) {
        if (strcmp(mappings[i].topic, t) == 0)
            return mappings[i].folder;
    }
    return NULL;
}

static void push_file(int fd, const char *path) {
    int in = open(path, O_RDONLY);
    if (in < 0) {
        dprintf(fd, "ERROR Could not open file: %s\n", path);
        return;
    }

    struct stat st;
    if (fstat(in, &st) < 0) {
        dprintf(fd, "ERROR fstat failed for %s\n", path);
        close(in);
        return;
    }

    const char *fname = strrchr(path, '/');
    fname = fname ? fname + 1 : path;

    char hdr[512];
    snprintf(hdr, sizeof(hdr), "%s %ld\n", fname, (long)st.st_size);
    send(fd, hdr, strlen(hdr), 0);

    char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(in, buf, sizeof(buf))) > 0) {
        send(fd, buf, n, 0);
    }
    close(in);
}

static void push_topic(int fd, const char *topic) {
    const char *dirpath = find_path(topic);
    if (!dirpath) {
        const char *err = "ERROR No Such Topic\n";
        send(fd, err, strlen(err), 0);
        return;
    }

    DIR *dp = opendir(dirpath);
    if (!dp) {
        const char *err = "ERROR Directory Access Failed\n";
        send(fd, err, strlen(err), 0);
        return;
    }

    int count = 0;
    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_REG) count++;
    }
    rewinddir(dp);

    char cntmsg[64];
    snprintf(cntmsg, sizeof(cntmsg), "%d\n", count);
    send(fd, cntmsg, strlen(cntmsg), 0);

    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_REG) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, de->d_name);
            push_file(fd, fullpath);
        }
    }
    closedir(dp);
}

static void *serve_client(void *arg) {
    int client = *(int *)arg;
    free(arg);

    char buf[BUF_SIZE];
    ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        if (strncmp(buf, "GET ", 4) == 0) {
            char *topic = buf + 4;
            char *nl = strchr(topic, '\n');
            if (nl) *nl = '\0';
            printf("Client requested topic: %s\n", topic);
            push_topic(client, topic);
        } else {
            const char *err = "ERROR Bad Request\n";
            send(client, err, strlen(err), 0);
        }
    }

    close(client);
    return NULL;
}

int main() {
    signal(SIGINT, handle_sigint);

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(LISTEN_PORT);

    if (bind(srv_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(srv_fd, 20) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Threaded server running on port %d\n", LISTEN_PORT);

    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        int *cli = (int*)malloc(sizeof(int));
        *cli = accept(srv_fd, (struct sockaddr *)&cliaddr, &clilen);
        if (*cli < 0) {
            perror("accept");
            free(cli);
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(cliaddr.sin_addr));
        pthread_t tid;
        pthread_create(&tid, NULL, serve_client, cli);
        pthread_detach(tid);
    }

    close(srv_fd);
    return 0;
}
