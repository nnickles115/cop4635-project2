/**
 * @file http_response.hpp
 * @brief This file contains the declaration of the HttpResponse class.
 * @details It is responsible for holding the HTTP response data.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "http_message.hpp"
#include "http_status.hpp"

#include <string>
#include <string_view>

/**
 * @brief Represents an HTTP response.
 * @note Inherits from HttpMessage.
 */
class HttpResponse : public HttpMessage {
public:
    // Constructors //
    HttpResponse() noexcept;

    // Getters //
    http::status::Code getStatus() const noexcept { return status; }
    bool isKeepAlive() const noexcept { return keepAlive; }
    
    // Setters //
    HttpResponse& setStatus(http::status::Code status) noexcept {
        this->status = status;
        return *this;
    }

    // Overrides //
    std::string getStatusLine() const noexcept override;
    void display() const override;

    // Functions //
    bool parse(std::string_view rawData);

private:
    // Variables //
    http::status::Code status;
    bool keepAlive;

    // Functions //
    bool parseStartLine(std::string_view line);
    void parseHeaders(std::string_view headersBlock);
    bool parseBody(std::string_view rawData, const size_t& bodyStart);
    void determineKeepAlive();
};

#endif // HTTP_RESPONSE_HPP