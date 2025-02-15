/**
 * @file http_request.cpp
 * @brief This file contains the definition of the HttpRequest class.
 * @details It is responsible for parsing raw HTTP request data 
 * into a structured object.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "http_request.hpp"
#include "http_method.hpp"
#include "logger.hpp"
#include "n_utils.hpp"

#include <iostream>
#include <sstream>

// Overrides //

/**
 * @brief Gets the HTTP method of the request.
 * @return The HTTP method.
 */
std::string HttpRequest::getStatusLine() const noexcept {
    std::ostringstream oss;
    oss << getMethod() << " " << getURI() << " " << getVersion();
    return oss.str();
}

/**
 * @brief Displays the HTTP request to the terminal.
 */
void HttpRequest::display() const {
    std::ostringstream oss;
    const int lineWidth = 24;

    oss << n_utils::io_style::seperator("HTTP REQUEST", '=', lineWidth) << "\n";
    oss << getStatusLine() << "\n";
    oss << n_utils::io_style::seperator("Headers", '-', lineWidth) << "\n";

    for(const auto& [key, value] : getAllHeaders()) {
        oss << key << ": " << value << "\n";
    }
    
    oss << n_utils::io_style::seperator("Body", '-', lineWidth) << "\n";
    oss << getBody() << "\n";
    oss << n_utils::io_style::seperator("", '=', lineWidth) << "\n";
    
    Logger::getInstance().print(oss.str());
}