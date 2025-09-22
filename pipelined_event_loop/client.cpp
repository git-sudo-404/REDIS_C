#include <iostream>
#include <vector>
#include <cstdint>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#define PORT 8080
#define BUFFER_SIZE 4096

// Helper function to read a specific number of bytes, handling partial reads
static bool read_all(int fd, void *buf, size_t n) {
    char *p = (char *)buf;
    size_t bytes_to_read = n;
    while (bytes_to_read > 0) {
        ssize_t bytes_read = read(fd, p, bytes_to_read);
        if (bytes_read <= 0) {
            // Error or connection closed
            return false;
        }
        p += bytes_read;
        bytes_to_read -= bytes_read;
    }
    return true;
}

// Helper function to send a length-prefixed message
static bool send_message(int fd, const std::string& msg) {
    uint32_t len = msg.length();
    uint32_t network_len = htonl(len);
    if (write(fd, &network_len, 4) != 4) return false;
    if (write(fd, msg.c_str(), len) != (ssize_t)len) return false;
    std::cout << "Sent: " << msg << std::endl;
    return true;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error");
        close(fd);
        return 1;
    }
    std::cout << "Successfully connected to server!" << std::endl;

    std::vector<std::string> msgs = {
        "Hello Server, message 1!",
        "This is message 2.",
        "And a third one for good measure.",
        "Last message, goodbye!"
    };

    for (const auto &msg : msgs) {
        if (!send_message(fd, msg)) {
            break;
        }
    }

    std::cout << "\nWaiting for server response..." << std::endl;

    // --- CORRECTED READ LOGIC ---
    while (true) {
        // 1. Read the 4-byte length prefix
        uint32_t network_len;
        if (!read_all(fd, &network_len, 4)) {
            break; // Connection closed or error
        }
        uint32_t len = ntohl(network_len);

        if (len > BUFFER_SIZE - 1) {
            std::cerr << "Error: Message too long!" << std::endl;
            break;
        }

        // 2. Read the message payload
        char buffer[BUFFER_SIZE];
        if (!read_all(fd, buffer, len)) {
            break; // Connection closed or error
        }
        buffer[len] = '\0'; // Null-terminate

        std::cout << "Received: " << buffer << std::endl;
    }

    close(fd);
    std::cout << "Connection closed." << std::endl;
    return 0;
}
