/**
 * @file socket.cpp
 * @brief This file contains the definition of the Socket class.
 * @details The Socket class serves as a wrapper around the socket file descriptor
 * to adhere to RAII principles.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "logger.hpp"
#include "socket.hpp"

#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include <thread>

// Constructors //

/**
 * @brief Constructs a new Socket object.
 * @param domain The socket domain (AF_INET).
 * @param type The socket type (SOCK_STREAM).
 * @param protocol The socket protocol.
 * @throws std::runtime_error if the socket creation fails.
 * @throws std::runtime_error if the socket options cannot be set.
 */
Socket::Socket(int domain, int type, int protocol) : socket_fd(-1) {
    socket_fd = ::socket(domain, type, protocol);
    if(socket_fd < 0) {
        throw std::runtime_error("Failed to create socket: " + std::string(std::strerror(errno)));
    }

    // Set socket options for reuse
    int optval = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        throw std::runtime_error("Failed to set socket options: " + std::string(std::strerror(errno)));
    }
}

/**
 * @brief Constructs a new Socket object from an existing socket file descriptor.
 * @param socket_fd The existing socket file descriptor.
 * @throws std::runtime_error if the socket file descriptor is invalid.
 */
Socket::Socket(int socket_fd) : socket_fd(socket_fd) {
    if(socket_fd < 0) {
        throw std::runtime_error("Invalid socket file descriptor");
    }
}

/**
 * @brief Destroys the Socket object.
 */
Socket::~Socket() noexcept {
    if(socket_fd >= 0) {
        Logger::getInstance().log("Closing socket.", Logger::LogLevel::DEBUG);
        close(socket_fd);
    }
}

/**
 * @brief Move constructor for the Socket object.
 * @param other The other Socket object to move.
 */
Socket::Socket(Socket&& other) noexcept : socket_fd(other.socket_fd) {
    other.socket_fd = -1;
}

/**
 * @brief Move assignment operator for the Socket object.
 * @param other The other Socket object to move.
 */
Socket& Socket::operator=(Socket&& other) noexcept {
    if(this != &other) {
        if(socket_fd >= 0) close(socket_fd);
        socket_fd = other.socket_fd;
        other.socket_fd = -1;
    }
    return *this;
}

// Setters //

/**
 * @brief Sets the socket to non-blocking mode.
 * @details This function uses the `fcntl()` system call to set the socket to non-blocking mode.
 * This allows the socket to be used with `poll()` or `epoll()` for asynchronous I/O.
 * @param enable `true` to enable non-blocking mode, `false` to disable.
 * @throws std::runtime_error if the socket flags cannot be set.
 */
void Socket::setNonBlocking(bool enable) {
    // F_GETFL = Get file status flags
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if(flags < 0) {
        throw std::runtime_error("Failed to get socket flags");
    }

    // O_NONBLOCK = Non-blocking mode
    if(enable) flags |= O_NONBLOCK; // Bitwise OR to enable
    else flags &= ~O_NONBLOCK;      // Bitwise AND with negation to disable

    // F_SETFL = Set file status flags
    if(fcntl(socket_fd, F_SETFL, flags) < 0) {
        throw std::runtime_error("Failed to set socket to non-blocking mode");
    }
}

// Functions //

/**
 * @brief Connects the socket to a server using `connect()` and `poll()` with a timeout.
 * @param server_addr The server address to connect to.
 * @param addrlen The length of the server address.
 * @param timeout_ms The timeout in milliseconds.
 * @throws std::runtime_error if the connection fails, times out, or is in an error state.
 */
void Socket::connect(const struct sockaddr* server_addr, socklen_t addrlen, int timeout_ms) {
    setNonBlocking(true);
    
    int result = ::connect(socket_fd, server_addr, addrlen);
    if(result == 0) return; // If connection established immediately, keep socket non-blocking

    // Check for EINPROGRESS (connection in progress)
    if(errno != EINPROGRESS) throw std::runtime_error(std::string(strerror(errno)));

    // Use poll() to wait for connection or timeout
    struct pollfd pfd;
    pfd.fd = socket_fd;
    pfd.events = POLLOUT; // Wait for writability (indicates connection completion)

    // Poll the socket
    int poll_result;
    do {
        poll_result = poll(&pfd, 1, timeout_ms);
    } while(poll_result < 0 && errno == EINTR);

    // Check for timeout
    if(poll_result == 0) {
        throw std::runtime_error("Connection timed out");
    } 
    else if(poll_result < 0) {
        throw std::runtime_error("Poll error during connect: " + std::string(strerror(errno)));
    }

    // Check for completion/error
    int socket_error = 0;
    socklen_t len = sizeof(socket_error);
    if(getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &socket_error, &len) < 0) {
        throw std::runtime_error("getsockopt failed: " + std::string(strerror(errno)));
    }

    if(socket_error != 0) {
        throw std::runtime_error("connect() failed: " + std::string(strerror(socket_error)));
    }
}

/**
 * @brief Receives data from the socket.
 * @param buf The buffer to store the data.
 * @param len The length of the buffer.
 * @param flags The flags to use for the receive operation.
 * @return The number of bytes received.
 * @throws std::system_error if the data cannot be received.
 */
ssize_t Socket::recv(void* buf, size_t len, int flags) const {
    ssize_t bytesRead = ::recv(socket_fd, buf, len, flags);
    if(bytesRead < 0) {
        Logger::getInstance().log("recv() returned -1.  errno: " + std::to_string(errno), Logger::LogLevel::DEBUG);
        if(errno != EAGAIN && errno != EWOULDBLOCK) {
            throw std::system_error(std::error_code(errno, std::system_category()), "Failed to receive data");
        }
    } 
    else {
        Logger::getInstance().log("recv() returned: " + std::to_string(bytesRead), Logger::LogLevel::DEBUG);
    }
    return bytesRead;
}

/**
 * @brief Sends data through the socket.
 * @param buf The buffer containing the data.
 * @param len The length of the buffer.
 * @param flags The flags to use for the send operation.
 * @return The number of bytes sent.
 * @throws std::system_error if the data cannot be sent.
 */
ssize_t Socket::send(const void* buf, size_t len, int flags) const {
    size_t totalSent = 0;
    const char* data = static_cast<const char*>(buf);
    while(totalSent < len) {
        ssize_t bytesSent = ::send(socket_fd, data + totalSent, len - totalSent, flags);
        Logger::getInstance().log("send() returned: " + std::to_string(bytesSent), Logger::LogLevel::DEBUG);
        if(bytesSent < 0) {
            Logger::getInstance().log("send() returned -1. errno: " + std::to_string(errno), Logger::LogLevel::DEBUG);
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Wait before retry
                continue; // Retry sending for non-fatal errors
            }
            throw std::system_error(std::error_code(errno, std::system_category()), "Failed to send data");
        }
        totalSent += bytesSent;
    }
    return totalSent;
}