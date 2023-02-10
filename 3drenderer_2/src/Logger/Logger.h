#pragma once

#include <vector>
#include <string>
#include <unordered_map>

enum LogType {
    LOG_NO_TYPE,
    LOG_INFO,
    LOG_ERROR,
};

enum LogCategory {
    LOG_CATEGORY_CAMERA,
    LOG_CATEGORY_CLIPPING,
};

struct LogEntry {
    LogType type;
    std::string message;
};

struct Logger {
	// Stores keys and values for each log category, along with all the messages
	// created for a particular frame for that category
    static std::unordered_map<LogCategory, std::vector<LogEntry>> messages;
    static void print(LogCategory category, const std::string& message);
    static void info(LogCategory category, const std::string& message);
    static void error(LogCategory category, const std::string& message);
};
