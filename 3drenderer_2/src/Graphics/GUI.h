#pragma once

#include <memory>
#include <vector>

struct LogEntry;
struct Window;
struct World;
union SDL_Event;

struct GUI
{
	std::shared_ptr<Window> window;
	std::shared_ptr<World> world;

	void initialize(std::shared_ptr<Window> app_window, std::shared_ptr<World> app_world);
	void process_input(SDL_Event &event);
	void render();
	void print_log_messages(std::vector<LogEntry>& log);
	void destroy();
};