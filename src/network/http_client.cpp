/**
 * @file http_client.cpp
 * @brief This file contains the definition of the HttpClient class.
 * @details This class is responsible for processing HTTP requests by serializing and sending them to the server,
 * then receiving and parsing the response and displaying it to the user.
 * 
 * @author Noah Nickles
 * @date 2/9/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "connection_manager.hpp"
#include "http_client.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "logger.hpp"

#include <arpa/inet.h>

#include <cstring>
#include <sstream>

// Constructors //

/**
 * @brief Constructs a new HttpClient object with a ConnectionManager reference.
 * @param connMgr The ConnectionManager object to use for sending and receiving data.
 * @note The ConnectionManager is not owned by the HttpClient, so it is passed by reference.
 */
HttpClient::HttpClient(ConnectionManager& connMgr) : connMgr(connMgr) {}

// Functions //

/**
 * @brief Processes an HTTP request by serializing and sending it to the server then receiving the response.
 * @param request The HttpRequest object to process.
 * @param ip The IP address of the server.
 * @param port The port number of the server.
 */
void HttpClient::processRequest(const HttpRequest& request, const std::string& ip, const std::string& port) {
    try {
        // Connect to the server if not already connected
        if(!connMgr.isConnected() && !connMgr.connect(ip, port)) {
            Logger::getInstance().log("Failed to connect to " + ip + ":" + port, Logger::LogLevel::ERROR);
            return;
        }
        Logger::getInstance().log("Connected to " + ip + ":" + port, Logger::LogLevel::INFO);

        // Serialize the request and send it
        std::string requestData = serializeRequest(request);
        Logger::getInstance().log("Serialized request.", Logger::LogLevel::DEBUG);
        if(!connMgr.send(requestData)) {
            Logger::getInstance().log("Failed to send request to " + ip + ":" + port, Logger::LogLevel::ERROR);
            return;
        }
        request.display(); // Display the formatted request

        // Receive the response and parse it
        auto responseData = connMgr.receive();
        if(!responseData.has_value()) {
            Logger::getInstance().log("Failed to receive response from " + ip + ":" + port, Logger::LogLevel::ERROR);
            return;
        }
        Logger::getInstance().log("Raw response received. Parsing....", Logger::LogLevel::DEBUG);
        HttpResponse response = parseResponse(responseData.value());
        response.display(); // Display the formatted response

        // Reset the connection if the response is not keep-alive
        if(!response.isKeepAlive()) {
            Logger::getInstance().log("Connection not kept alive. Disconnecting.", Logger::LogLevel::INFO);
            connMgr.disconnect();
        }
    }
    catch(const std::exception& e) {
        Logger::getInstance().log("Failed to process request: " + std::string(e.what()), Logger::LogLevel::ERROR);
    }
}

// Helpers //

/**
 * @brief Composes an HTTP request string from an HttpRequest object.
 * @param request The HttpRequest object to compose.
 * @return The serialized HTTP request string.
 */
std::string HttpClient::serializeRequest(const HttpRequest& request) const {
    // Serialize the request line
    std::ostringstream requestStream;
    requestStream << request.getStatusLine() << "\r\n";

    // Serialize the headers
    for(const auto& [key, value] : request.getAllHeaders()) {
        requestStream << key << ": " << value << "\r\n";
    }
    requestStream << "\r\n"; // Blank line between headers and body

    // Serialize the body if it exists
    if(!request.getBody().empty()) {
        requestStream << request.getBody();
    }

    // Return the serialized request
    return requestStream.str();
}

/**
 * @brief Parses an HTTP response string into an HttpResponse object.
 * @param responseData The HTTP response string to parse.
 * @return The parsed HttpResponse object.
 */
HttpResponse HttpClient::parseResponse(const std::string& responseData) const {
    HttpResponse response;
    if(!response.parse(responseData)) {
        throw std::runtime_error("Failed to parse HTTP response.");
    }
    return response;
}