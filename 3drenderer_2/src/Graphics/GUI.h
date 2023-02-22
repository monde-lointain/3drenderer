#pragma once

#include <memory>
#include <vector>

struct LogEntry;
struct Window;
struct World;
union SDL_Event;

struct GUI
{
	void initialize(Window* window_, World* world_);
	void process_input(SDL_Event& event);
	void render();
	static void print_log_messages(std::vector<LogEntry>& log);
	static void destroy();

	Window* window;
	World* world;
};