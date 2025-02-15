/**
 * @file input_handler.hpp
 * @brief This file contains the declaration of the InputHandler class.
 * @details This class is responsible for handling user input and displaying menus to the user
 * and using the HttpClient and ConnectionManager classes to send and receive HTTP requests.
 * 
 * @author Noah Nickles
 * @date 2/9/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "http_method.hpp"

#include <csignal>
#include <functional>
#include <iostream>
#include <optional>
#include <string>

#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

// Externs //
extern volatile std::sig_atomic_t signal_received;

// Forward Declarations //
class ConnectionManager;
class HttpClient;
class HttpRequest;

/**
 * @brief The InputHandler class is responsible for handling user input and displaying menus to the user
 * and using the HttpClient and ConnectionManager classes to send and receive HTTP requests.
 */
class InputHandler {
public:
    // Constructors //
    InputHandler(HttpClient& client, ConnectionManager& connMgr);

    // Lifecycle //
    void run();

private:
    // State Machine //
    enum class InputState {
        MAIN_MENU,
        REQUEST_MENU,
        HELP_MENU,
        ENTER_IP,
        ENTER_PORT,
        ENTER_URI,
        ENTER_POST_BODY,
        EXIT
    };
    void switchToState(const InputState& state);

    // Menus //
    void displayMainMenu();
    void displayRequestMenu();
    void displayHelpMenu();

    // Input Handlers //
    void handleMainMenuInput();
    void handleRequestMenuInput();
    void handleHelpMenuInput();

    // Input Readers //
    std::optional<std::string> readInput(
        const std::string& prompt,
        const std::function<bool(const std::string&)>& validation,
        const std::string& invalidErrorMessage
    );
    bool readIP();
    bool readPort();
    bool readURI();
    bool readPostBody();

    // Input Validators //
    bool checkValidIP(const std::string& ip);
    bool checkValidPort(const std::string& port);
    bool checkValidURI(const std::string& uri);

    // Utilities //
    void printMessage(const std::string& message) const;
    void clearScreen() const { std::system("clear"); }  //{ std::cout << "\033[2J\033[1;1H"; }

    // Connection Helpers //
    void quickConnect();
    void clearConnection();
    bool checkConnection();
    bool sendAndValidate(const http::method::Method& method, const std::string& uri, const std::string& body = "");

    // Request Building //
    HttpRequest buildRequest(const http::method::Method& method, const std::string& uri, const std::string& body) const;

    // Dependencies //
    HttpClient& client;
    ConnectionManager& connMgr;

    // State //
    InputState currentState;
    InputState previousState;

    // Constants //
    static constexpr int MAX_PORT = 65535;
    static constexpr int MIN_PORT = 1;
    const std::string ESC_KEY = "\033";

    // Variables //
    std::string ip;
    std::string port;
};

#endif // INPUT_HANDLER_HPP