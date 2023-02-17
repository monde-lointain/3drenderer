#pragma once

#include <memory>

struct Renderer;
struct Window;
struct World;
union SDL_Event;
/**
 * Handles receiving input for all non-GUI game objects and notifies them when
 * to change their state based on an input
 */

struct PlayerController
{
	void initialize(std::shared_ptr<Window> app_window,
		std::shared_ptr<World> app_world,
		std::shared_ptr<Renderer> app_renderer);
	void process_input(SDL_Event& event);

	std::shared_ptr<Window> window;	// Window clicked, window released, etc
	std::shared_ptr<World> world;	// Updating camera movement, ...
	std::shared_ptr<Renderer> renderer; // Setting the render mode
};
