#include "Application.h"

#include <iostream>

#include <tracy/tracy/Tracy.hpp>

#include "Controller/PlayerController.h"
#include "Graphics/GUI.h"
#include "Logger/Logger.h"
#include "Renderer/Renderer.h"
#include "Viewport/Viewport.h"
#include "Window/Window.h"
#include "World/World.h"

#ifdef _MSC_VER // Windows
#include <SDL.h>
#else // Linux
#include <SDL2/SDL.h>
#endif

Application::Application()
{
	std::cout << "Application constructor called.\n";

	controller = std::make_shared<PlayerController>();
	world = std::make_shared<World>();
	window = std::make_shared<Window>();
	viewport = std::make_shared<Viewport>();
	renderer = std::make_shared<Renderer>();
	gui = std::make_shared<GUI>();
}

Application::~Application()
{
	std::cout << "Application destructor called.\n";
}

void Application::initialize()
{
	window->initialize(viewport); // Initializes SDL and the SDL window and renderer
	gui->initialize(window, world); // Creates the ImGui context and sets up for SDL
	renderer->initialize(window, viewport, world); // Initializes the framebuffer and z buffer and assigns to the renderer the viewport, window and world pointers

	running = true;
}

void Application::setup() const
{
	world->load_level(viewport); // Creates the camera, light, all the starting models...
	controller->initialize(window, world, renderer); // Gives the controller all the relevant game objects it needs to access
}

void Application::run()
{
	setup();


	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);

	// We want the FPS to be fixed to the speed of the user's monitor refresh
	window->seconds_per_frame = 1.0f / (float)display_mode.refresh_rate;
	window->current_fps = 1.0f / window->seconds_per_frame;

	uint64 start, end;
	uint32 ms_to_wait;
	float time_elapsed, time_elapsed_for_frame;
	uint64 frame_end;

	while (running)
	{
		start = SDL_GetPerformanceCounter();
		input();
		update();
		render();
		FrameMark; // for tracy
		end = SDL_GetPerformanceCounter();

		time_elapsed = (float)(end - start) / (float)SDL_GetPerformanceFrequency();

		// If we still have time after updating the frame, wait to advance to
		// the next one
		if (time_elapsed < window->seconds_per_frame)
		{
			ms_to_wait = (uint32)((window->seconds_per_frame - time_elapsed) * 1000.0f);
			SDL_Delay(ms_to_wait);
		}

		// Compute the current FPS based on the frame time
		frame_end = SDL_GetPerformanceCounter();
		time_elapsed_for_frame = (float)(frame_end - start) / (float)SDL_GetPerformanceFrequency();

		window->current_fps = 1.0f / time_elapsed_for_frame;

		Logger::print(LOG_CATEGORY_PERF_COUNTER, "FPS: " + std::to_string(window->current_fps));
	}
}

void Application::destroy() const
{
	renderer->destroy(); // Frees the framebuffer, z buffer and framebuffer SDL texture
	gui->destroy(); // Destroys the imgui SDL context
	window->destroy(); // Destroys SDL window, renderer and SDL itself
}

void Application::input()
{
	ZoneScoped; // for tracy

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		// NOTE: Not for this engine, but potentially for future engines, use
		// different switch statements for different control schemes? Maybe the
		// controls are different for one game mode from another and we can
		// choose a different switch statement to pick based on a game mode
		// variable or something

		// Closing the window
		switch (event.type)
		{
			case SDL_QUIT:
			{
				running = false;
				break;
			}
			case SDL_KEYDOWN:
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
					break;
				}
			}
		}
		// Imgui SDL input handling
		gui->process_input(event);
		// Player controller SDL input handling
		controller->process_input(event);
	}
}

void Application::update() const
{
	world->update();
}

void Application::render() const
{
	renderer->render();
	gui->render();
	Renderer::display_frame();
}
