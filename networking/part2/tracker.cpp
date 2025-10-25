#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 23456
#define BUFFER_SIZE 1024
#define MAX_PEERS 100
#define MAX_TOPIC_LEN 64
#define MAX_PEER_ADDR_LEN 64

typedef struct {
    char ip[MAX_PEER_ADDR_LEN];
    int port;
    char topic[MAX_TOPIC_LEN];
} PeerInfo;

PeerInfo peers[MAX_PEERS];
int peer_count = 0;
pthread_mutex_t peers_mutex = PTHREAD_MUTEX_INITIALIZER;

// ---------------- ADD PEER ----------------
void add_peer(const char *ip, int port, const char *topic) {
    pthread_mutex_lock(&peers_mutex);

    // Check if peer already exists -> update topic
    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peers[i].ip, ip) == 0 && peers[i].port == port) {
            strncpy(peers[i].topic, topic, MAX_TOPIC_LEN - 1);
            peers[i].topic[MAX_TOPIC_LEN - 1] = '\0';
            printf("Updated peer %s:%d with topic %s\n", ip, port, topic);
            pthread_mutex_unlock(&peers_mutex);
            return;
        }
    }

    // Add new peer if space
    if (peer_count < MAX_PEERS) {
        strncpy(peers[peer_count].ip, ip, MAX_PEER_ADDR_LEN - 1);
        peers[peer_count].ip[MAX_PEER_ADDR_LEN - 1] = '\0';
        peers[peer_count].port = port;
        strncpy(peers[peer_count].topic, topic, MAX_TOPIC_LEN - 1);
        peers[peer_count].topic[MAX_TOPIC_LEN - 1] = '\0';
        peer_count++;
        printf("Added peer %s:%d with topic %s\n", ip, port, topic);
    } else {
        printf("Peer list full, cannot add %s:%d\n", ip, port);
    }

    pthread_mutex_unlock(&peers_mutex);
}

// ---------------- QUERY TOPIC ----------------
void get_peers_for_topic(const char *topic, char *response, size_t max_len) {
    pthread_mutex_lock(&peers_mutex);
    response[0] = '\0';

    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peers[i].topic, topic) == 0) {
            char entry[128];
            snprintf(entry, sizeof(entry), "%s:%d ", peers[i].ip, peers[i].port);
            strncat(response, entry, max_len - strlen(response) - 1);
        }
    }

    pthread_mutex_unlock(&peers_mutex);

    if (strlen(response) == 0) {
        snprintf(response, max_len, "NONE");
    }
}

// ---------------- CLIENT HANDLER ----------------
void *client_handler(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        close(client_fd);
        return NULL;
    }
    buffer[bytes] = '\0';

    // Protocol:
    // REGISTER <port> <topic>\n
    // QUERY <topic>\n
    if (strncmp(buffer, "REGISTER ", 9) == 0) {
        char topic[MAX_TOPIC_LEN];
        int port;

        // Get client IP
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        getpeername(client_fd, (struct sockaddr *)&addr, &addr_len);
        char *client_ip = inet_ntoa(addr.sin_addr);

        if (sscanf(buffer + 9, "%d %63s", &port, topic) == 2) {
            add_peer(client_ip, port, topic);
            const char *msg = "REGISTERED\n";
            send(client_fd, msg, strlen(msg), 0);
        } else {
            const char *msg = "ERROR Invalid REGISTER\n";
            send(client_fd, msg, strlen(msg), 0);
        }

    } else if (strncmp(buffer, "QUERY ", 6) == 0) {
        char topic[MAX_TOPIC_LEN];
        if (sscanf(buffer + 6, "%63s", topic) == 1) {
            char response[BUFFER_SIZE];
            get_peers_for_topic(topic, response, sizeof(response));
            strcat(response, "\n");
            send(client_fd, response, strlen(response), 0);
        } else {
            const char *msg = "ERROR Invalid QUERY\n";
            send(client_fd, msg, strlen(msg), 0);
        }
    } else {
        const char *msg = "ERROR Unknown command\n";
        send(client_fd, msg, strlen(msg), 0);
    }

    close(client_fd);
    return NULL;
}

// ---------------- MAIN ----------------
int main() {
    int server_fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Tracker listening on port %d\n", PORT);

    while (1) {
        int *client_fd = (int*)malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&addr, &addr_len);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
