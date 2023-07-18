#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MAX_DATA_SIZE 1024

int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("socket creation failed");
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8085);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("connection failed");
        close(clientSocket);
        return 1;
    }

    char message[MAX_DATA_SIZE];
    while (1)
    {
        printf("Enter a message (or 'quit' to exit): ");
        fgets(message, MAX_DATA_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "quit") == 0)
        {
            break;
        }

        if (send(clientSocket, message, strlen(message), 0) == -1)
        {
            perror("send failed");
            close(clientSocket);
            return 1;
        }

        char buffer[MAX_DATA_SIZE];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead == -1)
        {
            perror("receive failed");
            close(clientSocket);
            return 1;
        }
        else if (bytesRead == 0)
        {
            printf("Server closed the connection.\n");
            close(clientSocket);
            return 1;
        }
        else
        {
            buffer[bytesRead] = '\0';
            printf("Received from server: %s\n", buffer);
        }
    }

    close(clientSocket);
    return 0;
}
