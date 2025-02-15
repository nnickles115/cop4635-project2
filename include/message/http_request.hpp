/**
 * @file http_request.hpp
 * @brief This file contains the declaration of the HttpRequest class.
 * @details It is responsible for parsing raw HTTP request data 
 * into a structured object.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

// =HTTP Request Documentation=======================================
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview       |
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods        |
// https://developer.mozilla.org/en-US/docs/Glossary/Request_header |
// ==================================================================

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "http_message.hpp"
#include "http_method.hpp"

#include <string>

/**
 * @brief Represents an HTTP request.
 * @note Inherits from HttpMessage.
 */
class HttpRequest : public HttpMessage {
public:
    // Getters //
    std::string getMethod() const noexcept { return method; }
    std::string getURI() const noexcept { return uri; }

    // Setters //
    HttpRequest& setMethod(const http::method::Method& method) { 
        this->method = http::method::toString(method); 
        return *this;
    }
    HttpRequest& setURI(const std::string& uri) {
        this->uri = uri; 
        return *this;
    }
    
    // Overrides //
    std::string getStatusLine() const noexcept override;
    void display() const override;

private:
    // Variables //
    std::string method;
    std::string uri;
};

#endif // HTTP_REQUEST_HPP