#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddress;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed!\n");
        return 1;
    }

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed! Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set up server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Connection failed! Error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to server!\n");

    char currentState[20];
    char guess;
    char message[100];

    // Game loop
    while (1) {
        // Receive the current game state from the server
        if (recv(clientSocket, currentState, sizeof(currentState), 0) <= 0) {
            printf("Connection closed by server.\n");
            break;
        }
        printf("Word: %s\n", currentState);

        // Get user's guess
        printf("Enter a letter: ");
        scanf(" %c", &guess);

        // Send guessed letter to the server
        send(clientSocket, &guess, sizeof(guess), 0);

        // Receive the game status message
        if (recv(clientSocket, message, sizeof(message), 0) <= 0) {
            printf("Connection closed by server.\n");
            break;
        }
        printf("%s\n", message);

        // Check if game ended
        if (strstr(message, "Game Over") || strstr(message, "Congratulations")) {
            break;
        }
    }

    // Close the socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
