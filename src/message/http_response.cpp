/**
 * @file http_response.cpp
 * @brief This file contains the definition of the HttpResponse class.
 * @details It is responsible for holding the HTTP response data.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "http_response.hpp"
#include "http_status.hpp"
#include "logger.hpp"
#include "n_utils.hpp"

#include <iostream>
#include <sstream>

// Constructors //

HttpResponse::HttpResponse() noexcept : status(http::status::Code::OK), keepAlive(true) {}

// Overrides //

/**
 * @brief Generates the HTTP status line (HTTP/1.1 200 OK).
 */
std::string HttpResponse::getStatusLine() const noexcept {
    std::ostringstream oss;
    oss << getVersion() << " " << static_cast<int>(status) << " " << http::status::toString(status);
    return oss.str();
}

// Functions //

/**
 * @brief Parse raw HTTP response data into a structured object.
 * @param rawData The raw HTTP response data.
 * @return `true` if parsing succeeded, `false` if invalid.
 */
bool HttpResponse::parse(std::string_view rawData) {
    // Parse the start line and validate
    size_t startLineEnd = rawData.find("\r\n");
    if(startLineEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed response: Missing start line end.", Logger::LogLevel::ERROR);
        return false;
    }
    std::string_view startLine = rawData.substr(0, startLineEnd);
    if(!parseStartLine(startLine)) {
        Logger::getInstance().log("Malformed response: Invalid start line.", Logger::LogLevel::ERROR);
        return false;
    }

    // Parse headers and validate
    size_t headersStart = startLineEnd + 2; // Move past "\r\n"
    size_t headersEnd = rawData.find("\r\n\r\n", headersStart);
    if(headersEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed response: Missing headers end.", Logger::LogLevel::ERROR);
        return false;
    }
    std::string_view headersBlock = rawData.substr(headersStart, headersEnd - headersStart);
    parseHeaders(headersBlock);

    // Determine if the connection should be kept alive
    determineKeepAlive();
    
    // Parse the body and validate
    size_t bodyStart = headersEnd + 4; // Move past "\r\n\r\n"
    if(!parseBody(rawData, bodyStart)) {
        Logger::getInstance().log("Malformed request: Invalid body.", Logger::LogLevel::ERROR);
        return false;
    }

    return true;
}

/**
 * @brief Determine if the connection should be kept alive.
 */
void HttpResponse::determineKeepAlive() {
    if(auto connectionHeader = getHeader("Connection")) {
        std::string connection = n_utils::str_manip::trim(*connectionHeader);
        std::string lowerValue = n_utils::str_manip::toLower(connection);
        keepAlive = (lowerValue == "keep-alive");
    }
}

/**
 * @brief Parse the response line (Version, Code, Reason).
 * @param line The response line (HTTP/1.1 200 OK).
 * @return `true` if valid, `false` if malformed.
 */
bool HttpResponse::parseStartLine(std::string_view line) {
    // Find the version
    size_t versionEnd = line.find(" ");
    if(versionEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed start line: Missing space after version.", Logger::LogLevel::ERROR);
        return false;
    }

    // Find the status code
    size_t statusCodeEnd = line.find(" ", versionEnd + 1);
    if(statusCodeEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed start line: Missing space after URI.", Logger::LogLevel::ERROR);
        return false;
    }

    // Extract the version and status code
    std::string versionStr = std::string(line.substr(0, versionEnd));
    std::string statusCodeStr = std::string(line.substr(versionEnd + 1, statusCodeEnd - versionEnd - 1));
    try {
        http::status::Code code = http::status::fromString(statusCodeStr);
        if(code == http::status::Code::INVALID) {
            Logger::getInstance().log("Invalid status code.", Logger::LogLevel::ERROR);
            return false;
        }
        setVersion(versionStr);
        setStatus(code);
    }
    catch(const std::exception& e){
        Logger::getInstance().log("Status code could not be parsed.", Logger::LogLevel::ERROR);
        return false;
    }

    return true;
}

/**
 * @brief Parse the request headers.
 * @param headersBlock The headers as a string_view.
 */
void HttpResponse::parseHeaders(std::string_view headersBlock) {
    size_t pos = 0;
    while(pos < headersBlock.size()) {
        size_t end = headersBlock.find("\r\n", pos);
        if(end == std::string_view::npos) {
            end = headersBlock.size();
        }
        std::string_view header = headersBlock.substr(pos, end - pos);
        pos = (end == headersBlock.size() ? end : end + 2); // Move past "\r\n" if present

        if(header.empty()) continue;

        size_t separator = header.find(":");
        if(separator != std::string_view::npos) {
            // Allow optional whitespace after the colon
            size_t valueStart = header.find_first_not_of(" ", separator + 1);
            std::string_view value = (valueStart != std::string_view::npos)
                ? header.substr(valueStart)
                : "";
            setHeader(header.substr(0, separator), value);
        }
    }
}

/**
 * @brief Parse the response body.
 * @param rawData The raw response data.
 * @param bodyStart The index where the body starts.
 * @return `true` if valid, `false` if malformed.
 */
bool HttpResponse::parseBody(std::string_view rawData, const size_t& bodyStart) {
    if(bodyStart < rawData.size()) {
        // Check for Content-Length (impl. Transfer-Encoding later)
        if(auto contentLengthHeader = getHeader("Content-Length")) {
            try {
                size_t contentLength = std::stoul(std::string(*contentLengthHeader));
                if(rawData.size() - bodyStart >= contentLength) {
                    setBody(rawData.substr(bodyStart, contentLength));
                } 
                else {
                    Logger::getInstance().log("Incomplete response body received.", Logger::LogLevel::ERROR);
                    return false;
                }
            } 
            catch(const std::invalid_argument& e) {
                Logger::getInstance().log("Invalid Content-Length header.", Logger::LogLevel::ERROR);
                return false;
            }
            catch(const std::out_of_range& e) {
                Logger::getInstance().log("Content-Length value out of range.", Logger::LogLevel::ERROR);
                return false;
            }
        } 
    }
    else {
        setBody("");
    }

    return true;
}

// Overrides //

/**
 * @brief Displays the HTTP response to the terminal.
 */
void HttpResponse::display() const {
    std::ostringstream oss;
    const int lineWidth = 24;

    oss << n_utils::io_style::seperator("HTTP RESPONSE", '=', lineWidth) << "\n";
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