#include "Window.h"

#include "../Viewport/Viewport.h"

#ifdef _MSC_VER // Windows
#include <SDL.h>
#else // Linux
#include <SDL2/SDL.h>
#endif

void Window::initialize(std::shared_ptr<Viewport> app_viewport)
{
	viewport = app_viewport;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			name,
			"Error initializing SDL.",
			nullptr
		);
		exit(1);
	}

	// Create the window
	window = SDL_CreateWindow(
		name,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		viewport->width,
		viewport->height,
		SDL_WINDOW_BORDERLESS
	);

	if (!window)
	{
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			name,
			"Error creating SDL window.",
			nullptr
		);
		SDL_Quit();
		exit(1);
	}

	// Create the SDL renderer
	renderer = SDL_CreateRenderer(
		window, 
		-1, 
		SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
	);

	if (!renderer)
	{
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			name,
			"Error creating SDL renderer.",
			nullptr
		);
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(1);
	}
}

void Window::destroy()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Window::clicked()
{
	if (!is_clicked)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
		is_clicked = true;
	}
}

void Window::released()
{
	if (is_clicked)
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		is_clicked = false;
	}
}
