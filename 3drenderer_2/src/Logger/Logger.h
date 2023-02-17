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
    LOG_CATEGORY_LIGHT,
    LOG_CATEGORY_PERF_COUNTER,
};

struct LogEntry {
    LogType type;
    std::string message;
};

struct Logger {
    ~Logger();

    static void print(LogCategory category, const std::string& message);
    static void info(LogCategory category, const std::string& message);
    static void error(LogCategory category, const std::string& message);
    static void reset();

	// Stores keys and values for each log category, along with all the messages
	// created for a particular frame for that category
    static std::unordered_map<LogCategory, std::vector<LogEntry>> messages;
};
