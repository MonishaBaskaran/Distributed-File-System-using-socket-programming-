#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define PORT 17002 // Port for Stext server
#define BUFFER_SIZE 1024

// Function to create necessary directories for the Stext server
void create_directory() {
    mkdir("stext", 0755);
}

void handle_client(int new_socket) {
    char path[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE];
    char filename[BUFFER_SIZE];
    char filepath[BUFFER_SIZE];
    char destination_path[BUFFER_SIZE];
    ssize_t bytes_read;

    // Clear the buffers
    memset(path, 0, sizeof(path));
    memset(command, 0, sizeof(command));
    memset(filename, 0, sizeof(filename));
    memset(destination_path, 0, sizeof(destination_path));

    // Read the command sent by Smain
    bytes_read = read(new_socket, path, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        perror("Failed to read from socket");
        return;
    }
    path[bytes_read] = '\0'; // Null-terminate the string

    printf("Received command: %s\n", path);
    sscanf(path, "%s %s %s", command, filename, destination_path);

    //Handle RMFILE command recieved from Smain
    if (strncmp(command, "rmfile", 6) == 0) {
        printf("Trying to delete file: %s\n", filename);

        // Attempt to delete the file
        if (remove(filename) == 0) {
            printf("File %s deleted successfully.\n", filename);
            char *response = "File deleted successfully by textserver.\n";
            send(new_socket, response, strlen(response), 0);
        } else {
            perror("Failed to delete file");
            char *response = "Failed to delete file on textserver.\n";
            send(new_socket, response, strlen(response), 0);
        }
    } 
    //Handle UFILE command recieved from Smain
    else if (strncmp(command, "ufile", 5) == 0) {
        printf("Trying to upload file: %s to %s\n", filename, destination_path);

        // Handle the file upload
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "stext/%s/%s", destination_path + 7, filename);

        // Create directories if needed
        char mkdir_command[BUFFER_SIZE];
        snprintf(mkdir_command, sizeof(mkdir_command), "mkdir -p stext/%s", destination_path + 7);
        system(mkdir_command);

        // Attempt to open the file in the client's directory
        FILE *local_file = fopen(filename, "r");
        if (local_file != NULL) {
            FILE *server_file = fopen(full_path, "w");
            if (server_file == NULL) {
                perror("Failed to create file on the server");
                fclose(local_file);
                return;
            }

            char buffer[BUFFER_SIZE];
            ssize_t n;
            while ((n = fread(buffer, sizeof(char), BUFFER_SIZE, local_file)) > 0) {
                fwrite(buffer, sizeof(char), n, server_file);
            }

            fclose(local_file);
            fclose(server_file);
            printf("File %s saved in Smain at %s.\n", filename, full_path);
        }
    } 
    //Handle DFILE command recieved from Smain.
    else if (strncmp(command, "dfile", 5) == 0) {
            printf("Trying to download File from: %s\n", filename);

                //extract filename from filepath
            char *pdffile = (strrchr(filename, '/') ? strrchr(filename, '/') + 1 : filename);
            //printf("filename -- %s\n",pdffile);
            // Create the full path to store the file
            char full_path[BUFFER_SIZE];
            snprintf(full_path, sizeof(full_path), "downloads/%s", pdffile);

            
            FILE *smain_file = fopen(filename, "r");

            if (smain_file != NULL) {
                // The file exists locally, so read its content and upload it to the server
                FILE *client_file = fopen(full_path, "w");
                if (client_file == NULL) {
                    perror("Failed to create file on the client");
                    fclose(smain_file);
                    return;
                }
        
                char buffer[BUFFER_SIZE];
                size_t n;
                while ((n = fread(buffer, sizeof(char), BUFFER_SIZE, smain_file)) > 0) {
                    fwrite(buffer, sizeof(char), n, client_file);
                }
        
                fclose(smain_file);
                fclose(client_file);
                printf("File %s downloaded in client at %s.\n", pdffile, full_path);
            }
        
    } 
    //Handle DISPLAY command recieved from Smain
    else if (strncmp(command, "display", 7) == 0) {
    
        printf("Executing display command\n");
        char buffer[BUFFER_SIZE] = {0};
        struct dirent *entry;
        printf("Trying to fetch: %s",filename);
        DIR *dp = opendir(filename);

        if (dp == NULL) {
            perror("Failed to open spdf directory");
            snprintf(buffer, sizeof(buffer), "Failed to open spdf directory");
            send(new_socket, buffer, strlen(buffer), 0);
            return;
        }

        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt")) {
                strncat(buffer, entry->d_name, sizeof(buffer) - strlen(buffer) - 1);
                strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
            }
        }
        closedir(dp);

        send(new_socket, buffer, strlen(buffer), 0);
    
    }
    
    
    else {
        printf("Unknown command: %s\n", command);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    // Create the necessary directories
    create_directory();
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Stext is listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        handle_client(new_socket);

        close(new_socket);
    }

    return 0;
}
