/**
 * @file http_client.hpp
 * @brief This file contains the declaration of the HttpClient class.
 * @details This class is responsible for processing HTTP requests by serializing and sending them to the server,
 * then receiving and parsing the response and displaying it to the user.
 * 
 * @author Noah Nickles
 * @date 2/9/2025
 * COP4635 Sys & Net II - Project 2
 */

#include <string>

#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

// Forward Declarations //
class ConnectionManager;
class HttpRequest;
class HttpResponse;

/**
 * @brief The HttpClient class is responsible for processing HTTP requests by serializing and sending them to the server,
 * then receiving and parsing the response and displaying it to the user.
 */
class HttpClient {
public:
    // Constructors //
    HttpClient(ConnectionManager& connMgr);

    // Functions //
    void processRequest(const HttpRequest& request, const std::string& ip, const std::string& port);

private:
    // Dependencies //
    ConnectionManager& connMgr;

    // Helpers //
    std::string serializeRequest(const HttpRequest& request) const;
    HttpResponse parseResponse(const std::string& responseData) const;
};

#endif // HTTP_CLIENT_HPP