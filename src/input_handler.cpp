/**
 * @file input_handler.cpp
 * @brief This file contains the definition of the InputHandler class.
 * @details This class is responsible for handling user input and displaying menus to the user
 * and using the HttpClient and ConnectionManager classes to send and receive HTTP requests.
 * 
 * @author Noah Nickles
 * @date 2/9/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "connection_manager.hpp"
#include "http_client.hpp"
#include "http_encoding.hpp"
#include "http_mime.hpp"
#include "http_request.hpp"
#include "input_handler.hpp"
#include "logger.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>

// Constructors //

/**
 * @brief Constructs a new InputHandler object with the given HttpClient and ConnectionManager dependencies.
 */
InputHandler::InputHandler(HttpClient& client, ConnectionManager& connMgr)
    : client(client), connMgr(connMgr),
      currentState(InputState::MAIN_MENU),
      previousState(InputState::MAIN_MENU)
{}

// State Machine //

/**
 * @brief Helper function to provide transition between states.
 */
void InputHandler::switchToState(const InputState& state) {
    previousState = currentState;
    currentState = state;
}

// Lifecycle //

/**
 * @brief Runs the input handler state machine.
 */
void InputHandler::run() {
    while(currentState != InputState::EXIT) {
        if(signal_received) {
            Logger::getInstance().log("Exiting the program...", Logger::LogLevel::INFO);
            switchToState(InputState::EXIT);
        }

        switch(currentState) {
            case InputState::MAIN_MENU:
                displayMainMenu();
                break;
            case InputState::REQUEST_MENU:
                displayRequestMenu();
                break;
            case InputState::HELP_MENU:
                displayHelpMenu();
                break;
            case InputState::ENTER_IP:
                if(readIP()) switchToState(InputState::ENTER_PORT);
                break;
            case InputState::ENTER_PORT:
                if(readPort()) {
                    if(checkConnection()) switchToState(InputState::REQUEST_MENU);
                    else switchToState(InputState::MAIN_MENU);
                }
                break;
            case InputState::ENTER_URI:
                if(readURI()) switchToState(InputState::ENTER_URI); // Allow user to keep sending GET requests
                break;
            case InputState::ENTER_POST_BODY:
                if(readPostBody()) switchToState(InputState::ENTER_POST_BODY); // Allow user to keep sending POST requests
                break;
            case InputState::EXIT:
                clearConnection(); // Clean up socket resources
                break;
            default:
                Logger::getInstance().log("Invalid input state.", Logger::LogLevel::ERROR);
                break;
        }
    }
}

// Menus //

/**
 * @brief Displays the main menu to the user.
 */
void InputHandler::displayMainMenu() {
    clearScreen();
    std::cout << "Welcome to the HTTP Client!\n\n";
    if(!ip.empty() && !port.empty()) {
        std::cout << "Selected server: " << ip << ":" << port << "\n\n";
    }
    else {
        std::cout << "No server is selected. Please connect to a new server to get started.\n\n";
    }
    std::cout << n_utils::io_style::seperator("Main Menu", '=', 24) << std::endl;
    std::cout << "1) Connect to a new server\n";
    std::cout << "2) Continue browsing\n";
    std::cout << "3) Set default server info\n";
    std::cout << "4) Disconnect from server\n";
    std::cout << "h) Help\n";
    std::cout << "q) Quit\n";
    std::cout << n_utils::io_style::seperator("", '=', 24) << std::endl;
    handleMainMenuInput();
}

/**
 * @brief Displays the request menu to the user.
 */
void InputHandler::displayRequestMenu() {
    clearScreen();
    std::cout << n_utils::io_style::seperator("Request Menu", '=', 24) << std::endl;
    std::cout << "1) Send a GET request\n";
    std::cout << "2) Send a POST request\n";
    std::cout << n_utils::io_style::seperator("", '=', 24) << std::endl;
    handleRequestMenuInput();
}

/**
 * @brief Displays the help menu to the user.
 */
void InputHandler::displayHelpMenu() {
    clearScreen();
    std::cout << n_utils::io_style::seperator("Help Menu", '=', 24) << std::endl;
    std::cout << "To navigate the menus, press the corresponding key behind the ')' and hit 'Enter'.\n";
    std::cout << "To return to the main menu, at any prompt, press the 'ESC' key and hit 'Enter'.\n";
    std::cout << n_utils::io_style::seperator("", '=', 24) << std::endl;
    std::cout << "\nEnter any key to continue...\n";
    handleHelpMenuInput();
}

// Input Handlers //

/**
 * @brief Handles user input for the main menu.
 */
void InputHandler::handleMainMenuInput() {
    std::cout << "Enter your choice: ";
    std::string choice;
    std::getline(std::cin, choice);
    if(signal_received) return;

    if(choice == "1") {
        clearScreen();
        clearConnection();
        switchToState(InputState::ENTER_IP);
    }
    else if(choice == "2") {
        clearScreen();
        if(ip.empty() || port.empty()) {
            printMessage("You are not connected to a server. Please connect first.\n");
            switchToState(InputState::MAIN_MENU);
        }
        else {
            switchToState(InputState::REQUEST_MENU);
        }
    }
    else if(choice == "3") {
        clearScreen();
        quickConnect();
        switchToState(InputState::MAIN_MENU);
    }
    else if(choice == "4") {
        if(ip.empty() || port.empty()) printMessage("You are not connected to a server.\n");
        else clearConnection();
        switchToState(InputState::MAIN_MENU);
    }
    else if(choice == "h") {
        switchToState(InputState::HELP_MENU);
    }
    else if(choice == "q") {
        Logger::getInstance().log("Exiting the program...", Logger::LogLevel::INFO);
        switchToState(InputState::EXIT);
    }
    else {
        printMessage("\nInvalid choice. Please try again.\n");
        switchToState(InputState::MAIN_MENU);
    }
}

/**
 * @brief Handles user input for the request menu.
 */
void InputHandler::handleRequestMenuInput() {
    std::cout << "Enter your choice: ";
    std::string choice;
    std::getline(std::cin, choice);
    if(signal_received) return;

    if(choice == "1") {
        clearScreen();
        switchToState(InputState::ENTER_URI);
    }
    else if(choice == "2") {
        clearScreen();
        switchToState(InputState::ENTER_POST_BODY);
    }
    else if(choice == ESC_KEY) {
        switchToState(InputState::MAIN_MENU);
    }
    else {
        printMessage("\nInvalid choice. Please try again.\n");
        switchToState(InputState::REQUEST_MENU);
    }
}

/**
 * @brief Handles user input for the help menu.
 */
void InputHandler::handleHelpMenuInput() {
    std::string choice;
    std::getline(std::cin, choice);
    if(signal_received) return;
    switchToState(InputState::MAIN_MENU);
}

// Input Readers //

/**
 * @brief Reads user input with a prompt and validation function.
 * @param prompt The prompt to display to the user.
 * @param validation The validation function to check if the input is valid.
 * @param invalidErrorMessage The message to display if the input is invalid.
 * @return The user input if valid, otherwise `std::nullopt`.
 */
std::optional<std::string> InputHandler::readInput(
    const std::string& prompt,
    const std::function<bool(const std::string&)>& validation,
    const std::string& invalidErrorMessage
) {
    std::string input;
    while(true) {
        std::cout << prompt;
        std::getline(std::cin, input); // Read a whole line
        if(signal_received) break;

        if(input == ESC_KEY) {
            switchToState(InputState::MAIN_MENU);
            return std::nullopt;
        }
        else if(validation(input)) {
            return input;
        }
        else {
            printMessage(invalidErrorMessage);
            input.clear();
        }
    }
    return std::nullopt;
}

/**
 * @brief Reads the IP address from the user.
 * @return `true` if the IP address is valid and set, otherwise `false`.
 */
bool InputHandler::readIP() {
    clearScreen();
    auto ip = readInput(
        "Enter the IP address of the server (ex: 127.0.0.1): ",
        [this](const std::string& ip) { return checkValidIP(ip); },
        "Invalid IP address format. Please try again."
    );
    if(!ip.has_value()) return false;

    if(!ip.value().empty()) {
        this->ip = ip.value();
        return true;
    }
    return false;
}

/**
 * @brief Reads the port number from the user.
 * @return `true` if the port number is valid and set, otherwise `false`.
 */
bool InputHandler::readPort() {
    clearScreen();
    auto port = readInput(
        "Enter the port number of the server (ex: 60001): ",
        [this](const std::string& port) { return checkValidPort(port); },
        "Invalid port number format. Please enter a number between 1 to 65535."
    );
    if(!port.has_value()) return false;

    clearScreen();
    if(!port.value().empty()) {
        this->port = port.value();
        return true;
    }
    return false;
}

/**
 * @brief Reads the URI from the user.
 * @return `true` if the URI is valid and set, otherwise `false`.
 */
bool InputHandler::readURI() {
    auto uri = readInput(
        "Enter the URI of the site (ex: /index.html): ",
        [this](const std::string& uri) { return checkValidURI(uri); },
        "Invalid URI format. Please try again."
    );
    if(!uri.has_value()) return false;

    if(!uri.value().empty()) return sendAndValidate(http::method::Method::GET, uri.value());
    return false;
}

/**
 * @brief Reads the post body from the user.
 * @return `true` if the post body is valid and set, otherwise `false`.
 */
bool InputHandler::readPostBody() {
    auto postData = readInput(
        "Enter the form post content: ",
        [this](const std::string& postBody) { return !postBody.empty(); },
        "Post cannot be empty. Please try again."
    );
    if(!postData.has_value()) return false;

    if(!postData.value().empty()) {
        postData = http::encoding::encode(postData.value()); // Encode any special characters
        postData = "comment=" + postData.value(); // Add form data key (only key on server)
        return sendAndValidate(http::method::Method::POST, "/submit", postData.value());
    }
    return false;
}

// Input Validators //

/**
 * @brief Checks if the IP address is valid.
 * @param ip The IP address to check.
 * @return `true` if the IP address is valid, otherwise `false`.
 */
bool InputHandler::checkValidIP(const std::string& ip) {
    // Setup sockaddr_in struct
    struct sockaddr_in sa;
    std::memset(&sa, 0, sizeof(sa));

    // Check if IP address is valid
    int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    return result == 1; // 1 means valid IP address
}

/**
 * @brief Checks if the port number is valid.
 * @param port The port number to check.
 * @return `true` if the port number is valid, otherwise `false`.
 */
bool InputHandler::checkValidPort(const std::string& port) {
    if(port.empty() || !std::isdigit(port.front())) return false;

    // Check if port is within valid range
    int portNum;
    try {
        portNum = std::stoi(port);
    }
    catch(const std::exception& e) {
        return false;
    }
    return portNum >= MIN_PORT && portNum <= MAX_PORT;
}

/**
 * @brief Checks if the URI is valid.
 * @param uri The URI to check.
 * @return `true` if the URI is valid, otherwise `false`.
 */
bool InputHandler::checkValidURI(const std::string& uri) {
    return !uri.empty() && uri[0] == '/';
}

// Utilities //

/**
 * @brief Prints a message to the user for one second before clearing the screen.
 * @param message The message to print.
 */
void InputHandler::printMessage(const std::string& message) const {
    clearScreen();
    std::cout << message << std::flush; // Flush to ensure output is displayed
    sleep(1);
    clearScreen();
}

// Connection Helpers //

/**
 * @brief Connects to the default server.
 */
void InputHandler::quickConnect() {
    std::string default_ip = "127.0.0.1";
    std::string default_port = "60001";

    // Check if already connected to default server
    if(ip == default_ip && port == default_port) {
        printMessage("Already connected to server: " + ip + ":" + port + "\n");
        return;
    }

    // Set default server info
    ip = default_ip;
    port = default_port;

    // Attempt connection
    if(!checkConnection()) {
        // Reset state if connection fails
        ip.clear();
        port.clear();
        connMgr.disconnect();
        printMessage("Failed to connect to default server.\n");
    }
}

/**
 * @brief Clears the connection info to the server and disconnects
 * if currently connected.
 */
void InputHandler::clearConnection() {
    ip.clear();
    port.clear();
    connMgr.disconnect();
}

/**
 * @brief Checks the connection to the server and lets the user know
 * if the connection was successful.
 */
bool InputHandler::checkConnection() {
    if(!connMgr.isConnected()) {
        if(!connMgr.connect(ip, port)) {
            printMessage("Failed to connect to " + ip + ":" + port + ".\n");
            return false;
        }
    }
    printMessage("Connection to " + ip + ":" + port + " was successful. Server is able to be browsed.\n");
    return true;
}

/**
 * @brief Sends and validates an HTTP request.
 * @param method The HTTP method to use.
 * @param uri The URI to request.
 * @param body The body of the request.
 * @return `true` if the request was sent and validated, otherwise `false`.
 */
bool InputHandler::sendAndValidate(const http::method::Method& method, const std::string& uri, const std::string& body) {
    HttpRequest request = buildRequest(method, uri, body);
    if(request.getStatusLine().empty()) return false;

    // Check if connection is lost and reconnect
    if(!connMgr.isConnected()) {
        if(!connMgr.connect(ip, port)) {
            printMessage("Failed to reconnect to " + ip + ":" + port + ".\n");
            return false;
        }
    }

    client.processRequest(request, ip, port);
    return true;
}

// Request Building //

/**
 * @brief Builds an HTTP request based on the method, URI, and body.
 * @param method The HTTP method to use.
 * @param uri The URI to request.
 * @param body The body of the request.
 * @return The built HTTP request object.
 */
HttpRequest InputHandler::buildRequest(const http::method::Method& method, const std::string& uri, const std::string &body) const{
    // Create and set values for all requests
    HttpRequest request;
    request.setMethod(method)
           .setURI(uri)
           .setHeader("Host", ip + ":" + port)
           .setHeader("User-Agent", "HTTP Client/1.1")
           .setHeader("Accept", "*/*");
    
    // Set extra headers based on request type
    if(method == http::method::Method::GET) {
        request.setHeader("Connection", "keep-alive");
    }
    else if(method == http::method::Method::POST) {
        request.setBody(body)
               .setHeader("Content-Type", http::mime::toString(http::mime::Media::APP_FORM))
               .setHeader("Content-Length", std::to_string(body.length()))
               .setHeader("Connection", "close");
    }
    return request;
}