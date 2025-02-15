/**
 * @file connection_manager.cpp
 * @brief This file contains the definition of the ConnectionManager class.
 * @details This class is responsible for managing the connection to the server, 
 * sending and receiving data, and handling the socket.
 * 
 * @author Noah Nickles
 * @date 2/9/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "connection_manager.hpp"
#include "logger.hpp"
#include "n_utils.hpp"
#include "socket.hpp"

#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <sstream>

// Constructors //

/**
 * @brief Constructs a new ConnectionManager object.
 */
ConnectionManager::ConnectionManager() : connected(false) {}

// Getters //

/**
 * @brief Checks if the ConnectionManager is connected to the server.
 * @return `true` if connected, `false` otherwise.
 */
bool ConnectionManager::isConnected() {
    if(!socket) return false; // No socket exists

    char buffer;
    ssize_t result = recv(socket->get(), &buffer, 1, MSG_PEEK | MSG_DONTWAIT);

    if(result == 0) {
        // Peer closed the connection
        connected = false;
    } 
    else if(result < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
        // Other errors mean the connection is broken
        connected = false;
    }

    return connected;
}

// Helpers //

/**
 * @brief Polls the socket for the specified events.
 * @param events The events to poll for (POLLIN or POLLOUT).
 * @param timeout_ms The timeout in milliseconds.
 * @return `true` if the socket is ready for the specified events, `false` otherwise.
 */
bool ConnectionManager::pollSocket(short events, int timeout_ms) const {
    if(!socket) return false; // No socket to poll

    // Configure pollfd struct
    struct pollfd pfd;
    pfd.fd = socket->get();
    pfd.events = events;

    // Poll the socket
    int poll_ret = poll(&pfd, 1, timeout_ms);
    if(poll_ret <= 0) {
        std::string eventStr = (events == POLLOUT) ? "writing" : "reading";
        Logger::getInstance().log("Socket not ready for " + eventStr + ".", Logger::LogLevel::DEBUG);
        return false;
    }
    return true;
}

// Functions //

/**
 * @brief Connects to the server with the specified IP address and port.
 * @param ip The IP address of the server.
 * @param port The port number of the server.
 * @return `true` if the connection was successful, `false` otherwise.
 */
bool ConnectionManager::connect(const std::string& ip, const std::string& port) {
    if(ip.empty() || port.empty()) return false; // Invalid IP or port

    if(socket) disconnect(); // Disconnect from previous connection

    // Setup socket and address
    socket = std::make_unique<Socket>(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(port));

    // Convert IP address to network byte order
    if(inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        Logger::getInstance().log("Invalid IP address: " + ip, Logger::LogLevel::ERROR);
        return false;
    }

    // Connect to the server
    Logger::getInstance().log("Attempting to connect to " + ip + ":" + port + "...", Logger::LogLevel::INFO);
    try {
        socket->connect((struct sockaddr*)&addr, sizeof(addr), TIMEOUT_MS);
        Logger::getInstance().log("Connection successful.", Logger::LogLevel::INFO);
        connected = true;
        return true;
    }
    catch(const std::runtime_error& e) {
        Logger::getInstance().log("Connection failed: " + std::string(e.what()), Logger::LogLevel::ERROR);
        return false;
    }
}

/**
 * @brief Disconnects from the server by resetting the socket 
 * and setting the connected flag to `false`.
 */
void ConnectionManager::disconnect() {
    socket.reset();
    connected = false;
}

/**
 * @brief Sends the specified data to the server.
 * @param data The data to send.
 * @return `true` if the data was sent successfully, `false` otherwise.
 */
bool ConnectionManager::send(const std::string& data) const {
    if(!socket) return false; // No socket to send data
    
    // Send the data, Socket class handles sending in chunks
    return socket->send(data.c_str(), data.size(), MSG_NOSIGNAL);
}

/**
 * @brief Receives the HTTP response data from the server.
 * @details This function reads headers until "\r\n\r\n" is encountered, then reads the body if Content-Length is present.
 * @return The received data if successful, `std::nullopt` otherwise.
 */
std::optional<std::string> ConnectionManager::receive() {
    if(!socket) return std::nullopt;

    char buffer[BUFFER_SIZE] = {0};
    std::string data;

    // Read headers until "\r\n\r\n" is encountered
    while(data.find("\r\n\r\n") == std::string::npos) {
        if(!isReadable()) return std::nullopt;
        ssize_t bytesRead = socket->recv(buffer, BUFFER_SIZE, 0);
        if(bytesRead <= 0) {
            Logger::getInstance().log("Failed to read headers.", Logger::LogLevel::ERROR);
            return std::nullopt;
        }
        data.append(buffer, bytesRead);
    }

    // Split headers and body
    size_t headerEnd = data.find("\r\n\r\n");
    auto headersStr = data.substr(0, headerEnd);
    std::string body = data.substr(headerEnd + 4);

    // If headers include Content-Length, read the rest
    size_t contentLength = 0;
    std::string lowerKey = n_utils::str_manip::toLower("Content-Length");
    if(auto contentLengthStr = n_utils::str_manip::parseHeaderValue(headersStr, lowerKey)) {
        try {
            contentLength = std::stoul(*contentLengthStr);
        }
        catch(const std::invalid_argument& e) {
            contentLength = 0;
        }
    }

    // Read remaining bytes if the body is shorter than Content-Length header
    while(contentLength > 0 && body.size() < contentLength) {
        if(!isReadable()) return std::nullopt;
        ssize_t bytesRead = socket->recv(buffer, BUFFER_SIZE, 0);
        if(bytesRead <= 0) break;
        body.append(buffer, bytesRead);
    }

    // Combine headers and body back
    return headersStr + "\r\n\r\n" + body;
}