#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#define PORT 8080

SOCKET new_socket;   // For storing the client connection
char selectedWord[20];  // For storing the word to guess

// List of predefined words
const char *words[] = {
    "elephant", "tiger", "giraffe", "kangaroo", "panda",
    "cheetah", "dolphin", "alligator", "hippopotamus", "rhinoceros",
    "penguin", "koala", "flamingo", "chimpanzee", "ostrich",
    "peacock", "crocodile", "squirrel", "leopard", "buffalo"
};

int main() {
    WSADATA wsaData;
    SOCKET server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed!\n");
        return 1;
    }

    // Initialize random seed and select a word
    srand(time(NULL));
    strcpy(selectedWord, words[rand() % 20]);
    printf("Selected Word for Hangman: %s (hidden from clients)\n", selectedWord);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed! Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed! Error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed! Error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server is running on port %d... Waiting for connections.\n", PORT);

    // Accept a client connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
        printf("Accept failed! Error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Connection established with client!\n");

    // Game Variables
    char hiddenWord[20];
    int attempts = 6; // Max wrong guesses

    // Initialize the hidden word
    int i;
    for (i = 0; i < strlen(selectedWord); i++) {
        hiddenWord[i] = '_';
    }
    hiddenWord[strlen(selectedWord)] = '\0';

    // Game loop
    while (attempts > 0 && strchr(hiddenWord, '_') != NULL) {
        // Send the current state
        send(new_socket, hiddenWord, sizeof(hiddenWord), 0);

        // Receive guessed letter
        char guess;
        recv(new_socket, &guess, sizeof(guess), 0);

        int correct = 0;
        int i;
        for (i = 0; i < strlen(selectedWord); i++) {
            if (selectedWord[i] == guess) {
                hiddenWord[i] = guess;
                correct = 1;
            }
        }

        // Handle wrong guess
        if (!correct) {
            attempts--;
        }

        // Check game status
        char message[100];
        if (attempts <= 0) {
            sprintf(message, "Game Over! The word was: %s", selectedWord);
            send(new_socket, message, sizeof(message), 0);
            break;
        } else if (strchr(hiddenWord, '_') == NULL) {
            sprintf(message, "Congratulations! You guessed the word: %s", selectedWord);
            send(new_socket, message, sizeof(message), 0);
            break;
        } else {
            sprintf(message, "Attempts left: %d", attempts);
            send(new_socket, message, sizeof(message), 0);
        }
    }

    // Close the sockets and clean up
    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();

    return 0;
}
