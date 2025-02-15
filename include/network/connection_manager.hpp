/**
 * @file connection_manager.hpp
 * @brief This file contains the declaration of the ConnectionManager class.
 * @details This class is responsible for managing the connection to the server, 
 * sending and receiving data, and handling the socket.
 * 
 * @author Noah Nickles
 * @date 2/9/2025
 * COP4635 Sys & Net II - Project 2
 */

#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include "socket.hpp"

#include <poll.h>

#include <memory>
#include <optional>
#include <string>

/**
 * @brief The ConnectionManager class is responsible for managing the connection to the server, 
 * sending and receiving data, and handling the socket.
 */
class ConnectionManager {
public:
    // Constructors //
    ConnectionManager();

    // Getters //
    bool isConnected();
    bool isWritable() const { return pollSocket(POLLOUT); }
    bool isReadable() const { return pollSocket(POLLIN); }

    // Functions //
    bool connect(const std::string& ip, const std::string& port);
    void disconnect();
    bool send(const std::string& data) const;
    std::optional<std::string> receive();

private:
    // Dependencies //
    std::unique_ptr<Socket> socket;

    // Helpers //
    bool pollSocket(short events, int timeout_ms = 50) const;

    // Constants //
    static constexpr int TIMEOUT_MS = 5000; // 5 seconds
    static constexpr int BUFFER_SIZE = 128 * 1024; // 128KB

    // Variables //
    bool connected;
};

#endif // CONNECTION_MANAGER_HPP