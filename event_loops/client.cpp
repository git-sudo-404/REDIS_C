#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <cstdint>
#include <iostream>

#define k_max_msg 4096

#define PORT 8080

int main() {
    // 1. Create the client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Error: socket()");
        return 1;
    }

    // 2. Define the server address to connect to
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    // INADDR_LOOPBACK is the IP for localhost (127.0.0.1)
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // 3. Connect to the server
    int rv = ::connect(client_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rv < 0) {
        perror("Error: connect()");
        close(client_fd);
        return 1;
    }

    std::cout << "Successfully connected to the server." << std::endl;

    // 4. Prepare the message with a 4-byte length prefix
    const char *msg_text = "HI I am Client";
    uint32_t msg_len = strlen(msg_text);
    
    // Create a dynamic buffer to hold the full message (header + body)
    std::vector<uint8_t> send_buffer;
    
    // Insert the 4-byte length header at the beginning of the buffer
    send_buffer.insert(send_buffer.end(), (uint8_t *)&msg_len, (uint8_t *)&msg_len + sizeof(uint32_t));
    
    // Append the actual message text after the header
    send_buffer.insert(send_buffer.end(), msg_text, msg_text + msg_len);

    // 5. Send the complete message (header + body) to the server
    //    We use .data() and .size() to get the raw buffer info
    rv = write(client_fd, send_buffer.data(), send_buffer.size());
    if (rv < 0) {
        perror("Error: write()");
        close(client_fd);
        return 1;
    }

    std::cout << "Sent message: '" << msg_text << "'" << std::endl;

    // 6. Read the echoed response from the server
    char read_buf[4 + k_max_msg]; // Make buffer large enough for response
    memset(read_buf, 0, sizeof(read_buf)); // Clear the buffer

    rv = read(client_fd, read_buf, sizeof(read_buf) - 1);
    if (rv < 0) {
        perror("Error: read()");
        close(client_fd);
        return 1;
    } else if (rv == 0) {
        std::cout << "Server closed the connection." << std::endl;
        close(client_fd);
        return 0;
    }
    
    // 7. Process and print the echoed response
    // The server echoes the full message including the header
    uint32_t echoed_len = 0;
    memcpy(&echoed_len, read_buf, sizeof(uint32_t));
    
    // The message text starts after the 4-byte header
    const char *echoed_text = read_buf + sizeof(uint32_t);

    std::cout << "Received echo. Length: " << echoed_len << ", Message: '" << echoed_text << "'" << std::endl;
    
    // 8. Clean up
    close(client_fd);

    return 0;
}

