#include "PlayerController.h"

#include "../Renderer/Camera.h"
#include "../Renderer/Renderer.h"
#include "../Window/Window.h"
#include "../World/World.h"

#ifdef _MSC_VER // Windows
#include <SDL.h>
#else // Linux
#include <SDL2/SDL.h>
#endif

void PlayerController::initialize(
	Window* app_window,
	World* app_world,
	Renderer* app_renderer
)
{
	window = app_window;
	world = app_world;
	renderer = app_renderer;
}

void PlayerController::process_input(SDL_Event& event) const
{
	switch (event.type)
	{
		// Clicking the window
		case SDL_MOUSEBUTTONDOWN:
		{
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				window->clicked();
				world->camera.input_mode = INPUT_ENABLED;
				world->camera.window_clicked = true;
				break;
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				window->released();
				world->camera.input_mode = MOUSE_INPUT_DISABLED;
				world->camera.window_clicked = false;
				world->camera.mouse_has_position = false;
				break;
			}
			break;
		}
		// case SDL_MOUSEMOTION:
		//{
		//	Graphics::drag_window(event);
		//	break;
		// }
		//  Adjusting the camera speed with the mouse
		case SDL_MOUSEWHEEL:
		{
			if (event.wheel.y > 0)
			{
				world->camera.speed += world->camera.SPEED_INC;
				world->camera.speed = glm::clamp(
					world->camera.speed,
					world->camera.MIN_SPEED, 
					world->camera.MAX_SPEED
				);
			}
			else if (event.wheel.y < 0)
			{
				world->camera.speed -= world->camera.SPEED_INC;
				world->camera.speed = glm::clamp(
					world->camera.speed,
					world->camera.MIN_SPEED,
					world->camera.MAX_SPEED
				);
			}
		}
		// Keyboard controls
		case SDL_KEYDOWN:
		{
			// Setting the render mode on the renderer
			if (event.key.keysym.sym == SDLK_1)
			{
				renderer->backface_culling = false;
				renderer->set_render_mode(VERTICES_ONLY);
				break;
			}
			if (event.key.keysym.sym == SDLK_2)
			{
				renderer->backface_culling = false;
				renderer->set_render_mode(WIREFRAME);
				break;
			}
			if (event.key.keysym.sym == SDLK_3)
			{
				renderer->backface_culling = false;
				renderer->set_render_mode(WIREFRAME_VERTICES);
				break;
			}
			if (event.key.keysym.sym == SDLK_4)
			{
				renderer->backface_culling = true;
				renderer->set_render_mode(SOLID);
				break;
			}
			if (event.key.keysym.sym == SDLK_5)
			{
				renderer->backface_culling = true;
				renderer->set_render_mode(SOLID_WIREFRAME);
				break;
			}
			if (event.key.keysym.sym == SDLK_6)
			{
				renderer->backface_culling = true;
				renderer->set_render_mode(TEXTURED);
				break;
			}
			if (event.key.keysym.sym == SDLK_7)
			{
				renderer->backface_culling = true;
				renderer->set_render_mode(TEXTURED_WIREFRAME);
				break;
			}
			if (event.key.keysym.sym == SDLK_n)
			{
				// Toggle
				// NOTE: Broken now due to the clipper :(
				// display_face_normals = !display_face_normals;
				break;
			}
			if (event.key.keysym.sym == SDLK_b)
			{
				// Toggle
				renderer->backface_culling = !renderer->backface_culling;
				break;
			}
			// Setting the shading mode
			if (event.key.keysym.sym == SDLK_f)
			{
				renderer->set_shading_mode(FLAT);
				break;
			}
			if (event.key.keysym.sym == SDLK_g)
			{
				renderer->set_shading_mode(GOURAUD);
				break;
			}
		/*
		* Camera controls
		* Note: The camera controls are handled by ORing together
		* different flags on the move state bitmask, which allows us to
		* easily handle multiple keypresses at once by simply setting
		* and removing bits from the mask
		*/
			if (event.key.keysym.sym == SDLK_w)
			{
				world->camera.set_move_state(FORWARD, true);
				break;
			}
			if (event.key.keysym.sym == SDLK_s)
			{
				world->camera.set_move_state(BACKWARD, true);
				break;
			}
			if (event.key.keysym.sym == SDLK_d)
			{
				world->camera.set_move_state(RIGHT, true);
				break;
			}
			if (event.key.keysym.sym == SDLK_a)
			{
				world->camera.set_move_state(LEFT, true);
				break;
			}
			if (event.key.keysym.sym == SDLK_q)
			{
				world->camera.set_move_state(DOWN, true);
				break;
			}
			if (event.key.keysym.sym == SDLK_e)
			{
				world->camera.set_move_state(UP, true);
				break;
			}
			// Setting the projection mode
			if (event.key.keysym.sym == SDLK_p)
			{
				world->camera.set_projection(PERSPECTIVE);
				break;
			}
			if (event.key.keysym.sym == SDLK_o)
			{
				world->camera.set_projection(ORTHOGRAPHIC);
				break;
			}
			break;
		}
		case SDL_KEYUP:
		{
			if (event.key.keysym.sym == SDLK_w)
			{
				world->camera.set_move_state(FORWARD, false);
				break;
			}
			if (event.key.keysym.sym == SDLK_s)
			{
				world->camera.set_move_state(BACKWARD, false);
				break;
			}
			if (event.key.keysym.sym == SDLK_d)
			{
				world->camera.set_move_state(RIGHT, false);
				break;
			}
			if (event.key.keysym.sym == SDLK_a)
			{
				world->camera.set_move_state(LEFT, false);
				break;
			}
			if (event.key.keysym.sym == SDLK_q)
			{
				world->camera.set_move_state(DOWN, false);
				break;
			}
			if (event.key.keysym.sym == SDLK_e)
			{
				world->camera.set_move_state(UP, false);
				break;
			}
		}
	}
}
