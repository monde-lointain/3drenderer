#pragma once

#include <memory>

struct GUI;
struct PlayerController;
struct Renderer;
struct Viewport;
struct Window;
struct World;

struct Application
{
	Application();
	~Application();

	/**
	 * Receives all SDL input for objects other than the GUI.Needs to be able
	 * to access all game objects that could possibly be influenced by user
	 * input(entities mostly).Keeps track of the general application
	 * state(render mode, window clicked, backface culling on, etc.)
	 */
	std::shared_ptr<PlayerController> controller;

	/**
	 * Contains all entities in the scene and the camera which holds
	 * fov/aspect/clip plane info and the view/projection matrices. Creation and
	 * deletion of entities happens here
	 */
	std::shared_ptr<World> world;

	/**
	 * Contains the SDL window and renderer, as well as information about the
	 * viewport width and height. Contain the current fps and target seconds per
	 * frame as well
	 */
	std::shared_ptr<Window> window;

	/**
	 * Contains the window width and height, allowing it to easily be passed
	 * around to classes that need it without having to pass around the entire
	 * window struct
	 */
	std::shared_ptr<Viewport> viewport;

	/**
	 * Stores a pointer to the world to get access to all the entities. Stores
	 * the framebuffer and z buffer, render mode...
	 */
	std::shared_ptr<Renderer> renderer;

	/**
	 * Stores a pointer to the window class to be able to access the windowand
	 * renderer. Has its own function for processing SDL input. Needs to be able
	 * to access world entities in order to send commands to create, transform
	 * and destroy them
	 */
	std::shared_ptr<GUI> gui;

	bool running;

	void initialize();
	void setup();
	void run();

	void input();
	void update();
	void render();

	void destroy();
};
