#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <winsock2.h>     // Windows sockets library
#include <ws2tcpip.h>     // Additional functions for sockets on Windows
#define CLOSESOCKET closesocket // Define close function for Windows
#else
#include <sys/socket.h>   // Sockets library for Unix-like systems
#include <netinet/in.h>   // Internet address family definitions
#include <arpa/inet.h>    // Utilities for address handling
#include <unistd.h>       // Close function for Unix-like systems
#include <netdb.h>        // Functions for hostname resolution
#define CLOSESOCKET close // Define close function for Unix-like systems
#define INVALID_SOCKET -1 // Define invalid socket for compatibility
#define SOCKET_ERROR -1   // Define socket error for compatibility
typedef int SOCKET;       // Define SOCKET type for compatibility
#endif

#include "protocol.h"

#define BUFFERSIZE 1024    // Maximum buffer size for communication
#define PROTOPORT 12345    // Port number for the server

void errorhandler(const char *error_message) {
    perror(error_message); // Print error message
    fprintf(stderr, "Error occurred: %s\n", error_message);
}

int main(void) {
#ifdef _WIN32
    WSADATA wsaData; // Windows-specific socket initialization structure
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        errorhandler("WSAStartup failed");
        return -1;
    }
#endif

    // Create a UDP socket
    SOCKET c_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (c_socket == INVALID_SOCKET) {
        errorhandler("Failed to create socket");
#ifdef _WIN32
        WSACleanup(); // Clean up Winsock on error
#endif
        return -1;
    }

    // Configure server address
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));  // Clear the structure
    sad.sin_family = AF_INET;      // Use IPv4 address family

    // Resolve the hostname using the system's DNS or hosts file
    const char *hostname = "passwdgen.uniba.it"; // Replace with the hostname configured in hosts file
    struct hostent *host = gethostbyname(hostname); // Resolve hostname
    if (host == NULL) {
        errorhandler("Failed to resolve hostname");
        CLOSESOCKET(c_socket); // Close the socket on error
#ifdef _WIN32
        WSACleanup();          // Clean up Winsock on error
#endif
        return -1;
    }

    // Copy the resolved IP address into the socket address structure
    sad.sin_addr = *((struct in_addr *)host->h_addr);
    sad.sin_port = htons(PROTOPORT); // Set the server port

    char request[BUFFERSIZE];  // Buffer to hold the client's request
    char response[BUFFERSIZE]; // Buffer to hold the server's response

    while (1) {
        // Prompt the user for input
        printf("Enter password type (n, a, m, u, s) and length >= 6 and <= 32 or 'q' to quit or 'h' for help: ");
        fgets(request, BUFFERSIZE, stdin); // Read user input
        request[strcspn(request, "\n")] = '\0'; // Remove trailing newline

        // Exit the client if the user enters 'q'
        if (strcmp(request, "q") == 0) {
            printf("Exiting client...\n");
            break;
        }

        // Send the user's request to the server
        if (sendto(c_socket, request, strlen(request), 0, (struct sockaddr*)&sad, sizeof(sad)) < 0) {
            errorhandler("Failed to send request to server");
            break;
        }

        // Receive the server's response
        socklen_t server_len = sizeof(sad);
        int bytes_received = recvfrom(c_socket, response, BUFFERSIZE - 1, 0, (struct sockaddr*)&sad, &server_len);
        if (bytes_received < 0) {
            errorhandler("Failed to receive response from server");
            break;
        }

        response[bytes_received] = '\0'; // Null-terminate the server's response

        // Display the received password or message from the server
        printf("Received password from server: %s\n", response);
    }

    // Close the socket
    CLOSESOCKET(c_socket);

#ifdef _WIN32
    WSACleanup(); // Clean up Winsock on Windows
#endif
    return 0; // Exit the program
}
