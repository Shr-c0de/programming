#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#define TRACKER_IP "127.0.0.1"
#define TRACKER_PORT 23456
#define BUFFER_SIZE 4096

char *topic;
char *shared_dir;
int peer_port;

/* Utility */
ssize_t send_all(int sock, const void *buf, size_t len) {
    size_t total = 0;
    const char *p = (const char *)buf;
    while (total < len) {
        ssize_t sent = send(sock, p + total, len - total, 0);
        if (sent <= 0) return -1;
        total += sent;
    }
    return total;
}

/* ----------------------------- Peer Server ----------------------------- */
void *peer_server(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return NULL;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(peer_port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return NULL;
    }

    printf("Peer server listening on port %d\n", peer_port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            close(client_fd);
            continue;
        }
        buffer[bytes] = '\0';

        if (strncmp(buffer, "GET ", 4) == 0) {
            char req_topic[128];
            sscanf(buffer + 4, "%127s", req_topic);

            if (strcmp(req_topic, topic) == 0) {
                DIR *dir = opendir(shared_dir);
                if (!dir) {
                    const char *msg = "ERROR Cannot open shared directory\n";
                    send_all(client_fd, msg, strlen(msg));
                    close(client_fd);
                    continue;
                }

                int file_count = 0;
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_type == DT_REG)
                        file_count++;
                }

                rewinddir(dir);

                char count_msg[64];
                snprintf(count_msg, sizeof(count_msg), "%d\n", file_count);
                send_all(client_fd, count_msg, strlen(count_msg));

                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_type == DT_REG) {
                        char filepath[512];
                        snprintf(filepath, sizeof(filepath), "%s/%s", shared_dir, entry->d_name);

                        int fd = open(filepath, O_RDONLY);
                        if (fd < 0)
                            continue;

                        struct stat st;
                        if (fstat(fd, &st) < 0) {
                            close(fd);
                            continue;
                        }

                        char header[512];
                        snprintf(header, sizeof(header), "%zu %s %ld\n",
                                 strlen(entry->d_name), entry->d_name, st.st_size);
                        send_all(client_fd, header, strlen(header));

                        char filebuf[BUFFER_SIZE];
                        ssize_t r;
                        while ((r = read(fd, filebuf, sizeof(filebuf))) > 0) {
                            if (send_all(client_fd, filebuf, r) < 0)
                                break;
                        }
                        close(fd);
                    }
                }

                closedir(dir);
            } else {
                const char *msg = "ERROR Topic not available\n";
                send_all(client_fd, msg, strlen(msg));
            }
        } else {
            const char *msg = "ERROR Invalid command\n";
            send_all(client_fd, msg, strlen(msg));
        }
        close(client_fd);
    }

    close(server_fd);
    return NULL;
}

/* ----------------------------- Tracker Interaction ----------------------------- */
void register_with_tracker() {
    int sockfd;
    struct sockaddr_in tracker_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    tracker_addr.sin_family = AF_INET;
    tracker_addr.sin_port = htons(TRACKER_PORT);
    inet_pton(AF_INET, TRACKER_IP, &tracker_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return;
    }

    snprintf(buffer, sizeof(buffer), "REGISTER %d %s\n", peer_port, topic);
    send_all(sockfd, buffer, strlen(buffer));

    ssize_t bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("Tracker response: %s", buffer);
    }

    close(sockfd);
}

void query_peers_for_topic(char *result, size_t size) {
    int sockfd;
    struct sockaddr_in tracker_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        strncpy(result, "ERROR\n", size);
        return;
    }

    tracker_addr.sin_family = AF_INET;
    tracker_addr.sin_port = htons(TRACKER_PORT);
    inet_pton(AF_INET, TRACKER_IP, &tracker_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr)) < 0) {
        perror("connect");
        close(sockfd);
        strncpy(result, "ERROR\n", size);
        return;
    }

    snprintf(buffer, sizeof(buffer), "QUERY %s\n", topic);
    send_all(sockfd, buffer, strlen(buffer));

    ssize_t bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        strncpy(result, buffer, size - 1);
        result[size - 1] = '\0';
    } else {
        strncpy(result, "ERROR\n", size);
    }

    close(sockfd);
}

/* ----------------------------- Peer Downloader ----------------------------- */
void download_from_peer(const char *peer_ip, int port) {
    int sockfd;
    struct sockaddr_in peer_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port);
    inet_pton(AF_INET, peer_ip, &peer_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return;
    }

    snprintf(buffer, sizeof(buffer), "GET %s\n", topic);
    send_all(sockfd, buffer, strlen(buffer));

    ssize_t bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        printf("Failed to receive file count from peer\n");
        close(sockfd);
        return;
    }

    buffer[bytes] = '\0';
    int num_files = atoi(buffer);
    if (num_files <= 0) {
        printf("No files or error from peer\n");
        close(sockfd);
        return;
    }

    printf("Downloading %d files from peer %s:%d\n", num_files, peer_ip, port);

    struct stat st = {0};
    if (stat(topic, &st) == -1) mkdir(topic, 0700);

    for (int i = 0; i < num_files; i++) {
        int pos = 0;
        while (1) {
            bytes = recv(sockfd, buffer + pos, 1, 0);
            if (bytes <= 0) {
                printf("Header recv error\n");
                close(sockfd);
                return;
            }
            if (buffer[pos] == '\n') {
                buffer[pos] = '\0';
                break;
            }
            pos++;
            if (pos >= BUFFER_SIZE - 1) {
                printf("Header too long\n");
                close(sockfd);
                return;
            }
        }

        size_t name_len;
        char filename[256];
        long filesize;
        if (sscanf(buffer, "%zu %255s %ld", &name_len, filename, &filesize) != 3) {
            printf("Invalid file header: %s\n", buffer);
            close(sockfd);
            return;
        }

        printf("Receiving %s (%ld bytes)\n", filename, filesize);
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", topic, filename);

        int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");
            close(sockfd);
            return;
        }

        long received = 0;
        while (received < filesize) {
            int to_read = (filesize - received < BUFFER_SIZE)
                              ? (filesize - received)
                              : BUFFER_SIZE;
            bytes = recv(sockfd, buffer, to_read, 0);
            if (bytes <= 0) {
                printf("Connection lost\n");
                close(fd);
                close(sockfd);
                return;
            }
            write(fd, buffer, bytes);
            received += bytes;
        }
        close(fd);
    }

    printf("Download complete.\n");
    close(sockfd);
}

/* ----------------------------- Main ----------------------------- */
int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        printf("Usage: %s <topic> <shared_directory> [peer_port]\n", argv[0]);
        return 1;
    }

    topic = argv[1];
    shared_dir = argv[2];
    peer_port = (argc == 4) ? atoi(argv[3]) : 34567;

    struct stat st;
    if (stat(shared_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("Invalid shared directory: %s\n", shared_dir);
        return 1;
    }

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, peer_server, NULL);

    register_with_tracker();

    char peer_list[BUFFER_SIZE];
    query_peers_for_topic(peer_list, sizeof(peer_list));
    printf("Peers with topic '%s':\n%s", topic, peer_list);

    if (strcmp(peer_list, "NONE\n") != 0 && strncmp(peer_list, "ERROR", 5) != 0) {
        char *line = strtok(peer_list, "\n");
        while (line) {
            char peer_ip[64];
            int port;
            if (sscanf(line, "%63[^:]:%d", peer_ip, &port) == 2) {
                if (port != peer_port) {
                    download_from_peer(peer_ip, port);
                }
            }
            line = strtok(NULL, "\n");
        }
    } else {
        printf("No peers found with topic %s\n", topic);
    }

    pthread_join(server_thread, NULL);
    return 0;
}
