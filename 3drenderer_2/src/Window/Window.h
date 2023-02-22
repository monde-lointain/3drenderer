#pragma once

#include <memory>

struct SDL_Renderer;
struct SDL_Window;
struct Viewport;

struct Window
{
	void initialize(Viewport* viewport_);
	void destroy();

	void clicked();
	void released();

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	Viewport* viewport;
	const char* name;
	float current_fps;
	float seconds_per_frame;
	bool is_clicked;
	//bool is_dragging;
};
