/**
 * @file config.hpp
 * @brief This file contains the declaration of the Config class.
 * @details This class is responsible for managing client configuration settings and
 * parsing command line arguments.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

// =Argument Parsing Documentation==========================
// https://www.man7.org/linux/man-pages/man3/getopt.3.html |
// =========================================================

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "logger.hpp"

#include <string>

/**
 * @brief The ConfigData struct contains the configuration settings for the client.
 */
struct ConfigData {
    bool debug = false;
};

/**
 * @brief The Config class is responsible for managing client configuration settings and
 * parsing command line arguments.
 */
class Config {
public:
    // Singleton //
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    // Constructors //
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // Getters //
    bool isDebug() const noexcept { return data.debug; }
    Logger::LogLevel determineLogLevel() const;
    
    // Functions //
    void loadConfig(int argc, char* argv[]);

private:
    // Constants //
    Config() : configLoaded(false) {};

    // Data //
    ConfigData data;
    bool configLoaded;

    // Parsing //
    void parseCommandLine(int argc, char* argv[]);
    void handleInvalidOption(int optopt, char* argv[]);
};

#endif // CONFIG_HPP