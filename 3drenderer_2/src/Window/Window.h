#pragma once

#include <memory>

struct SDL_Renderer;
struct SDL_Window;
struct Viewport;

struct Window
{
	void initialize(std::shared_ptr<Viewport> app_viewport);
	void destroy();

	void clicked();
	void released();

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	const char* name;
	std::shared_ptr<Viewport> viewport;
	float current_fps;
	float seconds_per_frame;
	bool is_clicked;
	//bool is_dragging;
};
