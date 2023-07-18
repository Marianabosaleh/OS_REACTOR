#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "st_reactor.h"

#define MAX_DATA_SIZE 1024

typedef struct {
    int fd;
    reactor_t *reactor;
} client_data_t;

void echo_handler(int fd, reactor_t *reactor) {
    char buffer[MAX_DATA_SIZE];
    int num_bytes = read(fd, buffer, MAX_DATA_SIZE - 1);

    if (num_bytes > 0) {
        buffer[num_bytes] = '\0';
        printf("Received from client %d: %s\n", fd, buffer);
        if (write(fd, buffer, num_bytes) < 0) {
            perror("Write failed");
        }
        if (strcmp(buffer, "quit") == 0) {
            stopReactor(reactor);
        }
    } else if (num_bytes == 0 || (num_bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
        close(fd);
        for (int i = 0; i < reactor->registry_size; i++) {
            if (reactor->registry[i].fd == fd) {
                printf("Client %d has left the chat.\n", fd);
                reactor->registry[i].fd = 0;
                break;
            }
        }
    } else {
        perror("Read failed");
    }
}

void accept_handler(int fd, reactor_t *reactor) {
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_fd = accept(fd, (struct sockaddr *)&client_address, &client_len);
    if (client_fd == -1) {
        perror("accept failed");
        return;
    }
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    addFd(reactor, client_fd, echo_handler);
    printf("New client connected: %d\n", client_fd);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd == -1) {
        perror("socket creation failed");
        return 1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8085);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        return 1;
    }

    reactor_t *reactor = createReactor();
    if (reactor == NULL) {
        return 1;
    }

    addFd(reactor, server_fd, accept_handler);
    startReactor(reactor);

    while (1) {
        char command[10];
        scanf("%s", command);
        if (strcmp(command, "quit") == 0) {
            stopReactor(reactor);
            break;
        }
    }

    free(reactor);

    return 0;
}
