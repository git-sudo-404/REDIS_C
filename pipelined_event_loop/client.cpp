#include <iostream>
#include <vector>
#include <cstdint>
#include <unistd.h>      // For read, write, close
#include <sys/socket.h>  // For socket functions
#include <arpa/inet.h>   // For inet_addr
#include <string.h>      // For memset
#include <string>        // For std::string

// Define a port and buffer size for clarity
#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    // --- 1. Create a socket ---
    // AF_INET: IPv4, SOCK_STREAM: TCP, 0: Default protocol
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        // Always check for errors after system calls
        perror("socket error");
        return 1;
    }

    // --- 2. Specify server address ---
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT); // Use htons to convert port to network byte order

    // FIX: You were setting the address to the port number.
    // Use inet_addr("127.0.0.1") for localhost (loopback).
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // Alternatively, for any interface on the host: INADDR_ANY,
    // or for loopback: INADDR_LOOPBACK. inet_addr is more explicit.

    // --- 3. Connect to the server ---
    int rv = connect(fd, (const struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rv < 0) {
        perror("connect error");
        close(fd); // Clean up the file descriptor
        return 1;
    }

    std::cout << "Successfully connected to server!" << std::endl;

    // --- 4. Send messages to the server ---
    std::vector<std::string> msgs = {
        "Hello Server, message 1!",
        "This is message 2.",
        "And a third one for good measure.",
        "Last message, goodbye!"
    };

    for (const auto &msg : msgs) {
        // FIX: Use .c_str() to get the C-style string and .length() for the size.
        // strlen() is not safe if the string contains null characters.
        ssize_t bytes_written = write(fd, msg.c_str(), msg.length());
        if (bytes_written < 0) {
            perror("write error");
            break; // Exit loop on error
        }
        std::cout << "Sent: " << msg << std::endl;
        sleep(1); // Small delay to see messages individually on the server
    }

    // --- 5. Read response from the server ---
    char buffer[BUFFER_SIZE] = {0}; // Use a simple char array as a buffer
    ssize_t bytes_read;

    std::cout << "\nWaiting for server response..." << std::endl;

    // FIX: The read loop was structured incorrectly.
    // This loop reads data until the server closes the connection (read returns 0)
    // or an error occurs (read returns -1).
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE - 1)) > 0) {
        // Null-terminate the received data to safely print it as a string
        buffer[bytes_read] = '\0';
        std::cout << "Received " << bytes_read << " bytes: " << buffer << std::endl;

        // Clear the buffer for the next read
        memset(buffer, 0, BUFFER_SIZE);
    }

    if (bytes_read == 0) {
        // This is the normal way to exit the loop: server closed the connection
        std::cout << "Server closed the connection." << std::endl;
    } else if (bytes_read < 0) {
        // An error occurred during read
        perror("read error");
    }

    // --- 6. Close the socket ---
    close(fd);
    std::cout << "Connection closed." << std::endl;

    return 0;
}

