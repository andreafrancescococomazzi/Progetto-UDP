#include <stdio.h>  // Include standard input/output library for functions like printf, perror, etc.
#include <stdlib.h>  // Include standard library for functions like malloc, free, rand, etc.
#include <string.h>  // Include string handling functions like strcpy, strlen, etc.
#include <time.h>  // Include time handling functions like time(), srand(), etc.

#ifdef _WIN32  // If the code is being compiled on Windows...
#include <winsock2.h>  // Include Windows-specific sockets library
#include <ws2tcpip.h>  // Include additional TCP/IP-related functionality
#else  // If the code is being compiled on a Unix-like system (Linux/macOS)...
#include <sys/socket.h>  // Include socket library for Unix-like systems
#include <netinet/in.h>  // Include Internet-related functionality for Unix-like systems
#include <arpa/inet.h>  // Include functions for working with Internet addresses
#include <unistd.h>  // Include Unix system calls like close()
#include <errno.h>  // Include library for accessing error codes
#include <netdb.h>  // Per getnameinfo() e struct addrinfo
#endif

#include "protocol.h"

#define BUFFERSIZE 1024  // Define the maximum buffer size for incoming/outgoing data.
#define PROTOPORT 12345  // Define the port number on which the server will listen.

void errorhandler(const char *message) {
    perror(message);  // Print the error message to stderr.
#ifdef _WIN32
    fprintf(stderr, "Error: %s (code: %d)\n", message, WSAGetLastError());  // Print Windows-specific error code.
#endif
}

void send_help(int socket, struct sockaddr_in *client_addr, socklen_t client_len) {
    const char *help_message =  // Define a message that explains how to use the server.
        "Usage: [type] [length]\n"
        "Types:\n"
    	"Password Generator Help Menu\n"
    	"Commands:\n"
    	"  h           :   show this help menu"
        "  n <length>  :   generate numeric password (digits only)\n"
        "  a <length>  :   generate alphabetic password (lowercase letters)\n"
        "  m <length>  :   generate mixed password (lowercase letters and numbers)\n"
        "  s <length>  :   generate secure password (uppercase, lowercase, numbers, symbols)\n"
        "  u <length>  :   generate unambiguous secure password (no similar-looking characters)\n"
    	"  q           :   quit application\n"

    	"LENGTH must be between 6 and 32 characters\n"

    	"Ambiguous characters excluded in 'u' option:\n"
    	" 0 O o (zero and letters O)\n"
    	" 1 l I i (one and letters l, I)\n"
    	"2 Z z (two and letter Z)\n"
    	" 5 S s (five and letter S)\n"
    	"8 B (eight and letter B)\n"

        "Example: n 8\n";
    sendto(socket, help_message, strlen(help_message), 0, (struct sockaddr *)client_addr, client_len);  // Send help message to the client.
}

static char random_char(const char *charset) {
    size_t charset_size = strlen(charset);  // Get the size of the character set.
    return charset[rand() % charset_size];  // Randomly pick a character from the charset.
}

void generate_numeric(char *password, int length) {
    const char *digits = "0123456789";  // Define the characters for a numeric password.
    for (int i = 0; i < length; i++) {
        password[i] = random_char(digits);  // Generate random numeric characters.
    }
    password[length] = '\0';  // Null-terminate the string.
}

void generate_alpha(char *password, int length) {
    const char *letters = "abcdefghijklmnopqrstuvwxyz";  // Define the characters for an alphabetic password.
    for (int i = 0; i < length; i++) {
        password[i] = random_char(letters);  // Generate random alphabetic characters.
    }
    password[length] = '\0';  // Null-terminate the string.
}

void generate_mixed(char *password, int length) {
    const char *mixed = "abcdefghijklmnopqrstuvwxyz0123456789";  // Define the characters for an alphanumeric password.
    for (int i = 0; i < length; i++) {
        password[i] = random_char(mixed);  // Generate random alphanumeric characters.
    }
    password[length] = '\0';  // Null-terminate the string.
}

void generate_secure(char *password, int length) {
    const char *secure =  // Define the characters for a secure password (letters, digits, and special characters).
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}<>?";
    for (int i = 0; i < length; i++) {
        password[i] = random_char(secure);  // Generate random characters from the secure set.
    }
    password[length] = '\0';  // Null-terminate the string.
}

void generate_unambiguous(char *password, int length) {
    const char *excluded_chars = "0Oo1lIi2Zz5Ss8B";  // Define characters to exclude (similar-looking characters).
    for (int i = 0; i < length; i++) {
        char char_choice;
        do {
            if (rand() % 2) {  // Randomly choose between letters and digits.
                char_choice = 'a' + rand() % 26;  // Random lowercase letter.
            } else {
                char_choice = '0' + rand() % 10;  // Random digit.
            }
        } while (strchr(excluded_chars, char_choice));  // Ensure the chosen character isn't excluded.
        password[i] = char_choice;  // Assign the valid character to the password.
    }
    password[length] = '\0';  // Null-terminate the string.
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;  // Declare Windows socket data structure.
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {  // Initialize Windows Sockets library.
        errorhandler("WSAStartup failed");  // Handle any error during initialization.
        return -1;
    }
#endif

    srand((unsigned)time(NULL));  // Seed the random number generator using the current time.

    int s_socket = socket(AF_INET, SOCK_DGRAM, 0);  // Create a UDP socket.
    if (s_socket < 0) {  // If socket creation failed...
        errorhandler("Failed to create socket");  // Handle error.
#ifdef _WIN32
        WSACleanup();  // Clean up the Windows Sockets library.
#endif
        return -1;
    }

    struct sockaddr_in sad, client_addr;  // Define server and client address structures.
    socklen_t client_len = sizeof(client_addr);  // Define length of client address.
    memset(&sad, 0, sizeof(sad));  // Initialize server address structure to 0.
    sad.sin_family = AF_INET;  // Set the address family to IPv4.
    sad.sin_addr.s_addr = INADDR_ANY;  // Bind the server to any available network interface.
    sad.sin_port = htons(PROTOPORT);  // Set the port to the defined PROTOPORT.

    if (bind(s_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {  // Bind the socket to the server address.
        errorhandler("Failed to bind socket");  // Handle error.
#ifdef _WIN32
        closesocket(s_socket);  // Close the socket.
        WSACleanup();  // Clean up Windows Sockets library.
#else
        close(s_socket);  // Close the socket for Unix-like systems.
#endif
        return -1;
    }

    printf("Server is waiting for a connection on port %d...\n", PROTOPORT);  // Inform the server is waiting for a request.

    char request[BUFFERSIZE], response[BUFFERSIZE];  // Declare buffers for the incoming request and outgoing response.

    while (1) {  // Infinite loop to handle client requests.
        int bytes_received = recvfrom(s_socket, request, BUFFERSIZE - 1, 0, (struct sockaddr *)&client_addr, &client_len);  // Receive data from the client.
        if (bytes_received < 0) {  // If receiving data failed...
            errorhandler("Failed to receive request from client");  // Handle error.
            // Close socket before exiting due to error.
#ifdef _WIN32
            closesocket(s_socket);  // Close socket for Windows.
            WSACleanup();  // Clean up the Windows Sockets library.
#else
            close(s_socket);  // Close socket for Unix-like systems.
#endif
            break;  // Exit the loop after error.
        }
        request[bytes_received] = '\0';  // Null-terminate the received request.

        // Log client information.
        char host[NI_MAXHOST];  // Buffer per il nome host
        int result = getnameinfo((struct sockaddr *)&client_addr, client_len,
                                 host, sizeof(host), NULL, 0, NI_NAMEREQD);

        if (result == 0) {
            printf("New request from %s:%d\n", host, ntohs(client_addr.sin_port));
        } else {
            printf("New request from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            fprintf(stderr, "getnameinfo failed: %s\n", gai_strerror(result));
        }


        if (strcmp(request, "h") == 0) {  // If the client requested help...
            send_help(s_socket, &client_addr, client_len);  // Send help message to the client.
            continue;  // Continue to the next loop iteration.
        }

        int length;
        char type;
        if (sscanf(request, "%c %d", &type, &length) != 2 || length <= 0) {  // Parse the type and length from the request.
            snprintf(response, sizeof(response), "Invalid request format or length\n");  // Prepare error response.
            sendto(s_socket, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_len);  // Send error response.
            continue;  // Continue to the next iteration.
        }

        if (length < 6 || length > 32) {  // Validate the password length.
            snprintf(response, sizeof(response), "Error: Password length must be between 6 and 32 characters.\n");  // Prepare error response.
            sendto(s_socket, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_len);  // Send error response.
            continue;  // Continue to the next iteration.
        }

        // Generate password based on the requested type.
        switch (type) {
            case 'n':
                generate_numeric(response, length);  // Numeric password generation.
                break;
            case 'a':
                generate_alpha(response, length);  // Alphabetic password generation.
                break;
            case 'm':
                generate_mixed(response, length);  // Alphanumeric password generation.
                break;
            case 's':
                generate_secure(response, length);  // Secure password generation.
                break;
            case 'u':
                generate_unambiguous(response, length);  // Unambiguous password generation.
                break;
            default:
                snprintf(response, sizeof(response), "Invalid password type\n");  // Invalid type response.
                break;
        }

        // Log the generated password on the server.
        printf("Generated password: %s\n", response);

        sendto(s_socket, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_len);  // Send the generated password to the client.
    }

#ifdef _WIN32
    closesocket(s_socket);  // Close the socket on Windows.
    WSACleanup();  // Clean up the Windows Sockets library.
#else
    close(s_socket);  // Close the socket on Unix-like systems.
#endif
    return 0;  // Return from the main function.
}
