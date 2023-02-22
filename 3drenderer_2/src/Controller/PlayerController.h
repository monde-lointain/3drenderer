#pragma once

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
	void initialize(
		Window* app_window,
		World* app_world,
		Renderer* app_renderer
	);
	void process_input(SDL_Event& event) const;

	Window* window;	// Window clicked, window released, etc
	World* world;	// Updating camera movement, ...
	Renderer* renderer; // Setting the render mode
};
