/**
 * @file main.cpp
 * @brief This file contains the entry point for the program.
 * @details It is responsible for creating the necessary objects and running the program.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "config.hpp"
#include "http_client.hpp"
#include "input_handler.hpp"
#include "logger.hpp"
#include "connection_manager.hpp"

#include <csignal>
#include <cstdlib>
#include <cstring>

// Global variable to track if a signal was received
volatile std::sig_atomic_t signal_received = 0;

/**
 * @brief Signal handler for system signals.
 * @param signum The signal number.
 * @details This function logs the signal received and exits the program.
 */
void signalHandler(int signum) {
    signal_received = signum;
    Logger::getInstance().print("\nReceived SIG" + std::string(sigabbrev_np(signum)));
}

/**
 * @brief Registers signals to be captured.
 */
void registerSignals() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Disable SA_RESTART to prevent interrupted system calls
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

/**
 * @brief The entry point for the program.
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 */
int main(int argc, char* argv[]) {
    // Register signal handlers
    registerSignals();

    // Load the configuration and set the log level
    Config::getInstance().loadConfig(argc, argv);
    Logger::getInstance().setLogLevel(Config::getInstance().determineLogLevel());

    try {
        // Create objects and inject dependencies by reference
        ConnectionManager connMgr;
        HttpClient client(connMgr);
        InputHandler inputHandler(client, connMgr);

        // Run the program
        inputHandler.run();
    }
    catch(const std::exception& e) {
        Logger::getInstance().log("An error occurred: " + std::string(e.what()), Logger::LogLevel::ERROR);
        return EXIT_FAILURE;
    }

    Logger::getInstance().log("Program exited successfully.", Logger::LogLevel::INFO);
    return EXIT_SUCCESS;
}