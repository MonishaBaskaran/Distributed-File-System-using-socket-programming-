/* Run this file using the command './filename PORT_NUMBER' Example './Smain 17004' */
/* PDF SERVER and TEXT SERVER PORT are defined globally.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>


#define BUFFER_SIZE 1024
#define PDF_SERVER_PORT 17001
#define TEXT_SERVER_PORT 17002

// Function to create necessary directories for the Smain server
void create_directories() {
    mkdir("smain", 0755);
    mkdir("spdf", 0755);
    mkdir("stext", 0755);
}

/******************************************** UFILE FUNCTIONS  **************************************************************************/

// Function to handle uploading of .c files to the Smain server
void handle_c_file_upload(int new_socket, const char *destination_path, const char *filename ) {

    char full_path[BUFFER_SIZE];

    // Remove the "~smain/" prefix from destination_path
    if (strncmp(destination_path, "~smain/", 7) == 0) {
        destination_path += 7;
    }

    // Create the full path to store the file
    snprintf(full_path, sizeof(full_path), "smain/%s/%s", destination_path, filename);

    // Create directories if needed
    char mkdir_command[BUFFER_SIZE];
    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir -p smain/%s", destination_path);
    system(mkdir_command);

    // Attempt to open the file in the client's directory
    FILE *local_file = fopen(filename, "r");
    if (local_file != NULL) {
        // The file exists locally, so read its content and upload it to the server
        FILE *server_file = fopen(full_path, "w");
        if (server_file == NULL) {
            perror("Failed to create file on the server");
            fclose(local_file);
            return;
        }
        // Copy content from local file to server file
        char buffer[BUFFER_SIZE];
        size_t n;
        while ((n = fread(buffer, sizeof(char), BUFFER_SIZE, local_file)) > 0) {
            fwrite(buffer, sizeof(char), n, server_file);
        }
 
        fclose(local_file);
        fclose(server_file);
        printf("File %s saved in Smain at %s.\n", filename, full_path);
    }

}

// Function to forward a file upload request to a remote server (pdf or text server)
void forward_ufile_to_server(const char *filename, const char *destination_path, const char *server_ip, int server_port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    int file_fd;
    ssize_t bytes_read;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    // Convert server IP address
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported \n");
        return;
    }
    // Connect to the remote server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection to server failed \n");
        return;
    }

    // Send filename and destination path to the server
    snprintf(buffer, sizeof(buffer), "%s %s %s", "ufile", filename, destination_path);
    send(sock, buffer, strlen(buffer), 0);

    close(sock);
    printf("File %s saved in Smain at %s\n", filename, destination_path);


}

// Function to handle the "ufile" command from the client
void handle_ufile_command(int new_socket, char *filename, char *destination_path) {
    
    filename[strcspn(filename, "\n")] = 0;
    destination_path[strcspn(destination_path, "\n")] = 0;
    
    // Determine file type and handle accordingly
    if (strstr(filename, ".c")) {
        printf("Storing .c file locally: %s\n", filename);
        // Handle storing .c file locally
        handle_c_file_upload(new_socket, destination_path, filename);
    } 
    
    else if (strstr(filename, ".pdf")) {
        printf("Forwarding .pdf file to Spdf server: %s\n", filename);
        // Forward the file to Spdf server
        forward_ufile_to_server( filename, destination_path, "127.0.0.1", PDF_SERVER_PORT);
    } 
    
    else if (strstr(filename, ".txt")) {
        printf("Forwarding .txt file to Stext server: %s\n", filename);
        // Forward the file to Stext server
        forward_ufile_to_server( filename, destination_path, "127.0.0.1", TEXT_SERVER_PORT);
    } 
    
    else {
        printf("Unsupported file type: %s\n", filename);
    }

    char *response = "File uploaded successfully.\n";
    send(new_socket, response, strlen(response), 0);
}

/******************************************** UFILE FUNCTIONS ENDS  **************************************************************************/

/******************************************** RMFILE FUNCTIONS  **************************************************************************/

// Function to forward a file delete request to a remote server (pdf or text server)
void forward_delete_request(const char *filename, const char *server_ip, int server_port) {
    
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    // Convert server IP address
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported \n");
        return;
    }
    // Connect to the remote server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection to server failed \n");
        return;
    }

    // Send filename and destination path to the server
    snprintf(buffer, sizeof(buffer), "%s %s %s", "rmfile", filename, "rmfile");
    send(sock, buffer, strlen(buffer), 0);

    close(sock);
    printf("File deleted from path %s.\n", filename);


}

// Function to handle the "rmfile" command from the client
void handle_rmfile_command(int new_socket, char *filepath) {
    
    filepath[strcspn(filepath, "\n")] = 0;
    char *command = "rmfile";
    
    // Determine file type and handle accordingly
    if (strstr(filepath, ".c")) {
        printf("Removing .c file locally: %s\n", filepath);
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "smain/%s", filepath + 7);
        
        if (remove(full_path) == 0) {
            printf("File %s deleted successfully\n", full_path);
        } else {
            printf("Failed to delete file %s",full_path);
        }
    }  
    
    else if (strstr(filepath, ".pdf")) {
        printf("Forwarding delete request for .pdf file to Spdf server: %s\n", filepath);
        // Forward the delete request to Spdf server
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "spdf/%s", filepath + 7);
        forward_delete_request(full_path, "127.0.0.1", PDF_SERVER_PORT);      
    } 
    
    else if (strstr(filepath, ".txt")) {
        
        printf("Forwarding delete request for .txt file to Stext server: %s\n", filepath);
        // Forward the delete request to Stext server
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "stext/%s", filepath + 7);
        forward_delete_request(full_path, "127.0.0.1", TEXT_SERVER_PORT);       
    }    
    
    else {
        printf("Unsupported file type: %s\n", filepath);
    }
    
    char *response = "File Deleted successfully.\n";
    send(new_socket, response, strlen(response), 0);
}

/******************************************** RMFILE FUNCTIONS ENDS **************************************************************************/

/******************************************** DFILE FUNCTIONS  **************************************************************************/

// Function to download a .c file to the client's directory
void download_c_file(int new_socket, char *filepath)
{
    //extract filename from filepath
    char *filename = (strrchr(filepath, '/') ? strrchr(filepath, '/') + 1 : filepath);
    
    // Create the download path in the client to store the file
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "downloads/%s", filename);

    // Attempt to open the file from Smain server's directory
    FILE *smain_file = fopen(filepath, "r");

    if (smain_file != NULL) {
         // Read its content and write it to the client's directory
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
        printf("File %s downloaded in client at %s.\n", filename, full_path);
    }
}

// Function to forward a file download request to a remote server (pdf or text server)
void forward_download_request(const char *filename, const char *server_ip, int server_port) {
    
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    int file_fd;
    ssize_t bytes_read;
     // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

     // Convert server IP address
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported \n");
        return;
    }
    // Connect to the remote server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection to server failed \n");
        return;
    }

    // Send filename and destination path to the server
    snprintf(buffer, sizeof(buffer), "%s %s %s", "dfile", filename, "dfile");
    send(sock, buffer, strlen(buffer), 0);

    close(sock);
    printf("File downloaded from %s.\n", filename);


}

// Function to handle the "dfile" command from the client
void handle_dfile_command(int new_socket, char *filepath) {
    filepath[strcspn(filepath, "\n")] = 0;
    char *command = "dfile";
    
    // Determine file type and handle accordingly
    if (strstr(filepath, ".c")) {
        printf("Downloading .c file locally: %s\n", filepath);
        // Create directories if needed
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "smain/%s", filepath + 7);
        //printf("Fullpath -- : %s\n", full_path);
        download_c_file(new_socket,full_path);
        
    }  
    
    else if (strstr(filepath, ".pdf")) {
        printf("Forwarding download request for .pdf file to Spdf server: %s\n", filepath);
        // Forward the download request to Spdf server
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "spdf/%s", filepath + 7);
        //printf("Fullpath -- : %s\n", full_path);
        forward_download_request(full_path, "127.0.0.1", PDF_SERVER_PORT);

       
    } 
    
    else if (strstr(filepath, ".txt")) {
        
        printf("Forwarding download request for .txt file to Stext server: %s\n", filepath);
        // Forward the download request to Stext server
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "stext/%s", filepath + 7);
        //printf("Fullpath -- : %s\n", full_path);
        forward_download_request(full_path, "127.0.0.1", TEXT_SERVER_PORT);
       
    }   
        
    else {
        printf("Unsupported file type: %s\n", filepath);
    }
    
    char *response = "File Downloaded successfully.\n";
    send(new_socket, response, strlen(response), 0);
}

/******************************************** DFILE FUNCTIONS ENDS  **************************************************************************/

/******************************************** DISPLAY FUNCTIONS  **************************************************************************/
// Function to get the list of .c files stored locally in the Smain server's directory
char* get_local_c_files(const char *directory) {
    DIR *dir;
    struct dirent *entry;
    char *file_list = (char*) malloc(BUFFER_SIZE * sizeof(char));
    memset(file_list, 0, BUFFER_SIZE);

    if ((dir = opendir(directory)) == NULL) {
        perror("Failed to open directory");
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".c")) {
            strcat(file_list, entry->d_name);
            strcat(file_list, "\n");
        }
    }
    
    closedir(dir);

    return file_list;
}

// Function to request the list of files from a remote server (pdf or text server)
char* request_files_from_server(const char *file_path, const char *server_ip, int server_port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char *file_list = (char*) malloc(BUFFER_SIZE * sizeof(char));
    memset(file_list, 0, BUFFER_SIZE);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address / Address not supported\n");
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection to server failed\n");
        return NULL;
    }

    // Send the command to request file list
    snprintf(buffer, sizeof(buffer), "%s %s %s", "display", file_path, "display");
    send(sock, buffer, strlen(buffer), 0);

    // Read the response containing the file list
    int valread = read(sock, file_list, BUFFER_SIZE - 1);
    if (valread < 0) {
        perror("Read error");
        free(file_list);
        return NULL;
    }
    file_list[valread] = '\0';  // Null-terminate the string

    close(sock);
    return file_list;
}

// Function to handle the "display" command from the client
void handle_display_command(int new_socket, char *pathname) {
    pathname[strcspn(pathname, "\n")] = 0; // Remove any trailing newline characters

    // Validate pathname
    if (strncmp(pathname, "~smain/", 7) != 0) {
        char *error_msg = "Invalid pathname. Must be within ~smain.\n";
        send(new_socket, error_msg, strlen(error_msg), 0);
        return;
    }

    // Get the list of local .c files
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "smain/%s", pathname + 7);
    char *c_files_list = get_local_c_files(full_path);

    // Request list of .pdf files from pdfserver
    char full_path_pdf[BUFFER_SIZE];
    snprintf(full_path_pdf, sizeof(full_path_pdf), "spdf/%s", pathname + 7);    
    char *pdf_files_list = request_files_from_server( full_path_pdf, "127.0.0.1", PDF_SERVER_PORT);

    // Request list of .txt files from textserver
    char full_path_txt[BUFFER_SIZE];
    snprintf(full_path_txt, sizeof(full_path_txt), "stext/%s", pathname + 7);
   char *txt_files_list = request_files_from_server( full_path_txt, "127.0.0.1", TEXT_SERVER_PORT);

    // Consolidate the lists
    char consolidated_list[BUFFER_SIZE * 3] = {0};
    if (c_files_list) {
        strcat(consolidated_list, c_files_list);
        strcat(consolidated_list, "\n");
        free(c_files_list);  // Free the allocated memory
    }
    if (pdf_files_list) {
        strcat(consolidated_list, pdf_files_list);
        strcat(consolidated_list, "\n");
        free(pdf_files_list);  // Free the allocated memory
    } 
    if (txt_files_list) {
        strcat(consolidated_list, txt_files_list);
        strcat(consolidated_list, "\n");
        free(txt_files_list);  // Free the allocated memory
    }

    // Send the consolidated list to the client
    send(new_socket, consolidated_list, strlen(consolidated_list), 0);
}

/******************************************** DISPLAY FUNCTIONS ENDS **************************************************************************/

// Function to process client requests
void prcclient(int new_socket) {

    char buffer[BUFFER_SIZE] = {0};
    const char *response = "Command executed by Smain";
    int bytes_read;
    
    // Read the command from the client
    while( (bytes_read = read(new_socket, buffer, BUFFER_SIZE))>0)
     {
        buffer[bytes_read] = '\0';  // Null-terminate the string
        printf("Smain received command: %s\n", buffer);

        // Handle the command
        char *command = strtok(buffer, " ");

        //check type of command and perform operations
        if (strcmp(command, "ufile") == 0) {
            char *filename = strtok(NULL, " ");
            char *destination_path = strtok(NULL, " ");
            if (filename && destination_path) {
                printf("Uploading file %s to %s\n", filename, destination_path);
                handle_ufile_command(new_socket, filename, destination_path);
            } else {
                printf("Invalid command format\n");
            }
            
        }

        else if (strcmp(command, "rmfile") == 0) {
            char *filepath = strtok(NULL, " ");
            if (filepath) {
                printf("Deleting file %s\n", filepath);
                handle_rmfile_command(new_socket, filepath);
            } else {
                printf("Invalid command format\n");
            }
            
        }

        else if (strcmp(command, "dfile") == 0) {
            char *filepath = strtok(NULL, " ");
            if (filepath) {
                printf("Downloading file %s\n", filepath);
                handle_dfile_command(new_socket, filepath);
            } else {
                printf("Invalid command format\n");
            }
            
        }

        else if (strcmp(command, "display") == 0) {
            char *pathname = strtok(NULL, " ");
            if (pathname) {
                printf("Displaying files in %s\n", pathname);
                handle_display_command(new_socket, pathname);
            } else {
                printf("Invalid command format\n");
            }
        }

        else {
            printf("Unknown command %s\n",command);
        }
        
        memset(buffer, 0, BUFFER_SIZE); //clear the buffer
    } 
    
    if (bytes_read == 0) {
        printf("Client disconnected.\n");       
    } else {
        perror("Read error in prcclient");        
    }

    close(new_socket);
}


int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Signal handler to reap zombie processes
    signal(SIGCHLD, SIG_IGN);

    // Create the necessary directories
    create_directories();

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Smain server is listening on port %d...\n", port);

    while (1) {
        // Accept a connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        printf("Client connected with Smain!\n");
        // Fork a child process to handle the client
        if (fork() == 0) {
            close(server_fd); // Child process doesn't need the listener
            prcclient(new_socket);
        } else {
            close(new_socket); // Parent process doesn't need the connected socket
        }
    }

    return 0;
}
