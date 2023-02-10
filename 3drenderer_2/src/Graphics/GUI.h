#pragma once

#include <vector>

struct LogEntry;
union SDL_Event;
struct SDL_Window;
struct SDL_Renderer;

struct GUI
{
	GUI() = default;
	~GUI() = default;

	bool init(SDL_Window* window, SDL_Renderer* renderer);
	void process_input(SDL_Event &event);
	void render();
	void print_log_messages(std::vector<LogEntry>& log);
	void destroy();
};

