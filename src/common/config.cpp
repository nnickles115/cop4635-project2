/**
 * @file config.cpp
 * @brief This file contains the definition of the Config class.
 * @details This class is responsible for managing client configuration settings and
 * parsing command line arguments.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

#include "config.hpp"
#include "n_utils.hpp"
#include "logger.hpp"

#include <getopt.h>
#include <unistd.h>

#include <cctype>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

// Functions //

/**
 * @brief Uses a flag to ensure the arguments are only parsed once.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 */
void Config::loadConfig(int argc, char* argv[]) {
    if(configLoaded) return;

    Config& instance = getInstance();
    instance.parseCommandLine(argc, argv);
}

// Getters //

/**
 * @brief Determines the log level based on the configuration.
 * @return The determined log level enum.
 */
Logger::LogLevel Config::determineLogLevel() const {
    if(data.debug) return Logger::LogLevel::DEBUG;
    return Logger::LogLevel::INFO;
}

// Parsing //

/**
 * @brief Parses command line arguments using getopt() and creates a Config object.
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return A Config object with the parsed data.
 * @throws std::invalid_argument if a required argument is missing a value.
 */
void Config::parseCommandLine(int argc, char* argv[]) {
    ConfigData parsedData;

    static struct option long_options[] = {
        {"debug",         no_argument,       0, 'd'}, // -d or --debug
        {0,               0,                 0,  0 }  // Required null terminator
    };

    // Parse command line arguments
    int opt, option_index, verbosityCount = 0;
    while((opt = getopt_long(argc, argv, "d", long_options, &option_index)) != -1) {
        switch(opt) {
            case 'd': verbosityCount++; parsedData.debug = true; break;
            case '?': handleInvalidOption(optopt, argv);         break;
        }
    }

    // Update the ConfigData struct with the parsed data
    data = parsedData;
}

/**
 * @brief Handles invalid command line options.
 * @param optopt The invalid option character.
 * @param argv The command line arguments.
 * @throws std::invalid_argument with an error message.
 */
void Config::handleInvalidOption(int optopt, char* argv[]) {
    if(optopt) {
        std::cerr << "Error: Unknown option -" << static_cast<char>(optopt) << std::endl;
    } 
    else if(optind > 0) {  // Ensure optind is valid before using it
        std::cerr << "Error: Invalid long option - " << argv[optind - 1] << std::endl;
    }
    throw std::invalid_argument("Invalid command-line argument.");
}