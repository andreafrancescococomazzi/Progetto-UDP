#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUFFERSIZE 1024   // Maximum buffer size for incoming/outgoing data.
#define PROTOPORT 12345      // Port number for the server.

#define MAX_PASSWORD_LENGTH 32  // Maximum password length allowed.
#define MIN_PASSWORD_LENGTH 6   // Minimum password length allowed.


enum PasswordType {
    NUMERIC = 'n',
    ALPHABETIC = 'a',
    ALPHANUMERIC = 'm',
    SECURE = 's',
    UNAMBIGUOUS = 'u'
};

// Function prototypes
void errorhandler(const char *message);
void send_help(int socket, struct sockaddr_in *client_addr, socklen_t client_len);
void generate_numeric(char *password, int length);
void generate_alpha(char *password, int length);
void generate_mixed(char *password, int length);
void generate_secure(char *password, int length);
void generate_unambiguous(char *password, int length);
static char random_char(const char *charset);

#endif // PROTOCOL_H
