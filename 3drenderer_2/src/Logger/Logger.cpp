#include "Logger.h"

std::unordered_map<LogCategory, std::vector<LogEntry>> Logger::messages;

Logger::~Logger()
{
	Logger::reset();
}

void Logger::reset()
{
	// Clear all the messages in each log
	for (auto& log : messages) {
		log.second.clear();
	}
}

void Logger::print(LogCategory category, const std::string& message)
{
	LogEntry entry;
	entry.type = LOG_INFO;
	entry.message = message;

	// Create a new vector if no messages exist for a category yet
	if (!messages.contains(category))
	{
		messages[category] = std::vector<LogEntry>();
	}

	messages[category].push_back(entry);
}

void Logger::info(LogCategory category, const std::string& message)
{
	LogEntry entry;
	entry.type = LOG_INFO;
	entry.message = "INFO: " + message;

	// Create a new vector if no messages exist for a category yet
	if (!messages.contains(category))
	{
		messages[category] = std::vector<LogEntry>();
	}

	messages[category].push_back(entry);
}

void Logger::error(LogCategory category, const std::string& message)
{
	LogEntry entry;
	entry.type = LOG_ERROR;
	entry.message = "ERROR: " + message;

	// Create a new vector if no messages exist for a category yet
	if (!messages.contains(category))
	{
		messages[category] = std::vector<LogEntry>();
	}

	messages[category].push_back(entry);
}
