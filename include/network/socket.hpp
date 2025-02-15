/**
 * @file socket.hpp
 * @brief This file contains the declaration of the Socket class.
 * @details The Socket class serves as a wrapper around the socket file descriptor
 * to adhere to RAII principles.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

// =Socket Programming Man Pages============================
// https://man7.org/linux/man-pages/man2/close.2.html      |
// https://man7.org/linux/man-pages/man2/connect.2.html    |
// https://man7.org/linux/man-pages/man2/recv.2.html       |
// https://man7.org/linux/man-pages/man2/send.2.html       |
// https://man7.org/linux/man-pages/man2/setsockopt.2.html |
// https://man7.org/linux/man-pages/man2/socket.2.html     |
// =========================================================

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>

/**
 * @brief The Socket class serves as a wrapper around the socket file descriptor 
 * to adhere to RAII principles.
 */
class Socket {
public:
    // Constructors //
    Socket(int domain, int type, int protocol);
    explicit Socket(int socket_fd);
    ~Socket() noexcept;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    // Getters //
    int get() const { return socket_fd; }
    bool isValid() const { return socket_fd >= 0; }
    
    // Setters //
    void setNonBlocking(bool enable);
    
    // Functions //
    void connect(const struct sockaddr* addr, socklen_t addrlen, int timeout_ms);
    ssize_t recv(void* buf, size_t len, int flags) const;
    ssize_t send(const void* buf, size_t len, int flags) const;

private:
    // Variables //
    int socket_fd;
};

#endif // SOCKET_HPP