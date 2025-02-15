/**
 * @file logger.hpp
 * @brief This file contains the declaration of the Logger class.
 * @details This class is a singleton that provides thread safe 
 * logging functionality.
 * 
 * @author Noah Nickles
 * @date 2/2/2025
 * COP4635 Sys & Net II - Project 2
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <mutex>
#include <string>
#include <string_view>

/**
 * @brief The Logger class is a singleton class that provides logging functionality.
 */
class Logger {
public:
    // Enum //
    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    // Singleton //
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // Constructors //
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Getters //
    LogLevel getLogLevel() const noexcept { return currentLevel; }

    // Setters //
    void setLogLevel(const LogLevel& level) noexcept { currentLevel = level; }
    
    // Logging //
    void log(std::string_view message, const LogLevel& level) noexcept;
    void log(std::string_view message, const LogLevel& level, std::ostream& out) noexcept;
    void print(std::string_view message) noexcept;

    // Helpers //
    std::string toString(const LogLevel& level) noexcept;
    LogLevel toEnum(const std::string& level) noexcept;

private:
    // Constructors //
    Logger(LogLevel level = LogLevel::INFO) noexcept : currentLevel(level) {};
    
    // Variables //
    std::mutex logMutex;
    LogLevel currentLevel;
};

#endif // LOGGER_HPP