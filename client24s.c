/* Run this file using the command './filename PORT_NUMBER' Example './client24s 17004' */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int sock = 0;
    struct sockaddr_in serv_addr;
    char command[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        printf("Enter command (ufile or dfile or rmfile or display or exit): ");
        if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
            printf("Error reading input\n");
            break;
        }
        
         // Exit the loop
        if (strncmp(command, "exit", 4) == 0) {
            break;
        }
        
        //If the user doesn't use any of the command, then print usage
        if (!((strncmp(command, "ufile", 5) == 0) || (strncmp(command, "dfile", 5) == 0) || (strncmp(command, "rmfile", 6) == 0) || (strncmp(command, "display", 7) == 0)))
        {
            printf("Usage\nufile [filename] [destination_path]\ndfile [filepath]\nrmfile [filepath]\ndtar [filetype]\ndisplay [path]\n");
            continue;
        }
        
        // Send user input to the server
        send(sock, command, strlen(command), 0);

        // Receive the response from the server
        int reply = read(sock, command, BUFFER_SIZE);
        if (reply > 0) {
            command[reply] = '\0';
            printf("%s\n", command);  // Print the server's response
        } else {
            printf("Server closed the connection\n");
            break;
        }
    }

    close(sock);

    return 0;
}
