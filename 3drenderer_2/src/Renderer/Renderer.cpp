#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Line/Line3D.h"
#include "../Logger/Logger.h"
#include "../Math/Math3D.h"
#include "../Mesh/Mesh.h"
#include "../Mesh/Triangle.h"
#include "../Utils/Colors.h"
#include "../Utils/debug_counters.h"
#include "../Utils/debug_helpers.h"
#include "../Utils/math_helpers.h"
#include "../Utils/string_ops.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <SDL.h>
#include <cassert>
#include <iostream>

Renderer::Renderer()
{
	std::cout << "Renderer constructor called.\n";
	is_running = false;
	seconds_per_frame = 0.0f;
	current_fps = 0.0f;
	render_mode = WIREFRAME;
	display_face_normals = false;
	backface_culling = false;
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.direction = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.update();
	projection_matrix = glm::mat4(0.0f);
}

Renderer::~Renderer()
{
	std::cout << "Renderer destructor called.\n";
}

void Renderer::setup()
{
	is_running = Graphics::initialize_window();
	Graphics::initialize_framebuffer();

	// Lock the mouse to the screen
	SDL_SetRelativeMouseMode(SDL_FALSE);

	// Initialize the camera
	camera.fov = 60.0f; 
	camera.aspect = (float)Graphics::viewport.width / (float)Graphics::viewport.height;
	camera.znear = 1.0f;
	camera.zfar = 10.0f;

	// Create the projection matrix
	projection_matrix = Math3D::create_projection_matrix(camera);

	// Create the view matrix
	glm::vec3 target = camera.position + camera.direction;
	view_matrix = glm::lookAt(camera.position, target, glm::vec3(0.0f, 1.0f, 0.0f));

	// Create the meshes in the scene
	Mesh* mesh = create_mesh("assets/models/cube/cubetest.obj");
	meshes.push_back(mesh);
}

void Renderer::process_input()
{
	BEGIN_TIMED_BLOCK(Input)
	SDL_Event event;

	/**
	 * Controls:
	 * 1-7: Toggle render mode
	 * Mouse x/y: Adjust camera pitch and yaw
	 * WS/DA/QE: Move camera in the direction of the forward/right/world up vectors
	 * N: Toggle normals
	 * B: Toggle backface culling
	 * ESC: Quit
	 */

	while (SDL_PollEvent(&event))
	{
		// Imgui SDL input handling
		Graphics::gui_process_input(event);

		switch (event.type)
		{
			// Closing the window
			case SDL_QUIT:
			{
				is_running = false;
				break;
			}
			// Moving the window
			case SDL_MOUSEBUTTONDOWN:
			{
				if (event.button.button == SDL_BUTTON_LEFT) {
					Graphics::window_clicked(event);
					break;
				}
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				if (event.button.button == SDL_BUTTON_LEFT) {
					Graphics::window_released();
					break;
				}
				break;
			}
			case SDL_MOUSEMOTION:
			{
				Graphics::drag_window(event);
				break;
			}
			// Keyboard controls
			case SDL_KEYDOWN:
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					is_running = false;
					break;
				}
				if (event.key.keysym.sym == SDLK_1)
				{
					set_render_mode(VERTICES_ONLY);
					break;
				}
				if (event.key.keysym.sym == SDLK_2)
				{
					set_render_mode(WIREFRAME);
					break;
				}
				if (event.key.keysym.sym == SDLK_3)
				{
					set_render_mode(WIREFRAME_VERTICES);
					break;
				}
				if (event.key.keysym.sym == SDLK_4)
				{
					set_render_mode(SOLID);
					break;
				}
				if (event.key.keysym.sym == SDLK_5)
				{
					set_render_mode(SOLID_WIREFRAME);
					break;
				}
				if (event.key.keysym.sym == SDLK_6)
				{
					set_render_mode(TEXTURED);
					break;
				}
				if (event.key.keysym.sym == SDLK_7)
				{
					set_render_mode(TEXTURED_WIREFRAME);
					break;
				}
				if (event.key.keysym.sym == SDLK_n)
				{
					// Toggle
					display_face_normals = !display_face_normals;
					break;
				}
				if (event.key.keysym.sym == SDLK_b)
				{
					// Toggle
					backface_culling = !backface_culling;
					break;
				}
				/*
				 * Note: The camera controls are handled by ORing together
				 * different flags on the move state bitmask, which allows us to
				 * easily handle multiple keypresses at once by simply setting
				 * and removing bits from the mask
				 */
				if (event.key.keysym.sym == SDLK_w)
				{
					camera.set_move_state(FORWARD, true);
					break;
				}
				if (event.key.keysym.sym == SDLK_s)
				{
					camera.set_move_state(BACKWARD, true);
					break;
				}
				if (event.key.keysym.sym == SDLK_d)
				{
					camera.set_move_state(RIGHT, true);
					break;
				}
				if (event.key.keysym.sym == SDLK_a)
				{
					camera.set_move_state(LEFT, true);
					break;
				}
				if (event.key.keysym.sym == SDLK_q)
				{
					camera.set_move_state(DOWN, true);
					break;
				}
				if (event.key.keysym.sym == SDLK_e)
				{
					camera.set_move_state(UP, true);
					break;
				}
				break;
			}
			case SDL_KEYUP:
			{
				if (event.key.keysym.sym == SDLK_w)
				{
					camera.set_move_state(FORWARD, false);
					break;
				}
				if (event.key.keysym.sym == SDLK_s)
				{
					camera.set_move_state(BACKWARD, false);
					break;
				}
				if (event.key.keysym.sym == SDLK_d)
				{
					camera.set_move_state(RIGHT, false);
					break;
				}
				if (event.key.keysym.sym == SDLK_a)
				{
					camera.set_move_state(LEFT, false);
					break;
				}
				if (event.key.keysym.sym == SDLK_q)
				{
					camera.set_move_state(DOWN, false);
					break;
				}
				if (event.key.keysym.sym == SDLK_e)
				{
					camera.set_move_state(UP, false);
					break;
				}
			}
		}
	}
	END_TIMED_BLOCK(Input)
}

void Renderer::update()
{
	BEGIN_TIMED_BLOCK(Update)
	// Update the position and rotation of the camera
	camera.update();

	// Update the view matrix
	glm::vec3 target = camera.position + camera.direction;
	view_matrix = glm::lookAt(camera.position, target, glm::vec3(0.0f, 1.0f, 0.0f));

	for (Mesh* &mesh : meshes)
	{
		// Initialize the model in the center of the screen
		glm::vec3 scale(1.0f);
		glm::vec3 rotation(0.0f);
		glm::vec3 translation(0.0f, 0.0f, 0.0f);
		x += 0.5f;

		// Create the world matrix for the model by concatenating rotation,
		// scaling, and transformation matrices
		glm::mat4 world_matrix =
			Math3D::create_world_matrix(scale, rotation, translation);

		// Concatenate the world and view matrices into the modelview matrix
		glm::mat4 modelview_matrix = view_matrix * world_matrix;

		// Update the positons of the gizmo and all triangles in 3D space
		transform_triangles(mesh, modelview_matrix);
		transform_gizmo(modelview_matrix);
	}
	END_TIMED_BLOCK(Update)
}

void Renderer::render()
{
	BEGIN_TIMED_BLOCK(Render)
	Graphics::clear_framebuffer(Colors::BLUE);
	Graphics::clear_z_buffer();

	// Render all triangles in the scene
	render_triangles_in_scene();
	// Clear the array of triangles
	triangles_in_scene.clear();

	// Render the gizmo
	render_gizmo();
	// Reset the gizmo to its default position
	gizmo.reset();

	Graphics::update_framebuffer();

	Graphics::render_gui();
	Graphics::render_frame();
	END_TIMED_BLOCK(Render)
}

void Renderer::run()
{
	setup();

	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);

	// We want the FPS to be fixed to the speed of the user's monitor refresh
	seconds_per_frame = 1.0f / (float)display_mode.refresh_rate;
	current_fps = 1.0f / seconds_per_frame;

	while (is_running)
	{
		BEGIN_TIMED_BLOCK(FrameLoop)
		uint64 start = SDL_GetPerformanceCounter();
		process_input();
		update();
		render();
		uint64 end = SDL_GetPerformanceCounter();
		END_TIMED_BLOCK(FrameLoop)

		float time_elapsed = (float)(end - start) / (float)SDL_GetPerformanceFrequency();

		// If we still have time after updating the frame, wait to advance to
		// the next one
		if (time_elapsed < seconds_per_frame)
		{
			uint32 ms_to_wait = (uint32)((seconds_per_frame - time_elapsed) * 1000.0f);
			SDL_Delay(ms_to_wait);
		}

		// Compute the current FPS based on the frame time
		uint64 frame_end = SDL_GetPerformanceCounter();
		float elapsed_time_for_frame = (float)(frame_end - start) / (float)SDL_GetPerformanceFrequency();

		current_fps = 1.0f / elapsed_time_for_frame;

		dump_cycle_counters();
	}
}

void Renderer::destroy()
{
	for (Mesh* &mesh : meshes)
	{
		delete mesh;
	}
	Graphics::close_window();
}

void Renderer::transform_triangles(Mesh* mesh, const glm::mat4& modelview_matrix)
{
	for (const Triangle& triangle : mesh->triangles)
	{
		// Initialize the new triangle
		Triangle transformed_triangle = triangle;

		// Transform the vertices by the model-view matrix
		for (glm::vec4& vertex : transformed_triangle.vertices)
		{
			Math3D::transform_point(vertex, modelview_matrix);
		}

		// Rotate the vertex normals
		for (glm::vec3& normal : transformed_triangle.normals)
		{
			Math3D::rotate_normal(normal, modelview_matrix);
		}

		// Compute the face normal of the triangle
		compute_face_normal(transformed_triangle);

		// Add the transformed triangle to the bin of triangles to be rendered
		triangles_in_scene.push_back(transformed_triangle);
	}
}

void Renderer::transform_gizmo(const glm::mat4& modelview_matrix)
{
	// Transform the bases and origin by the modelview matrix
	for (Line3D& basis : gizmo.bases)
	{
		Math3D::transform_point(basis.points[0], modelview_matrix);
		Math3D::transform_point(basis.points[1], modelview_matrix);
	}
}

void Renderer::project_gizmo()
{
	for (Line3D& basis : gizmo.bases)
	{
		Math3D::project_point(basis.points[0], projection_matrix, Graphics::viewport, camera);
		Math3D::project_point(basis.points[1], projection_matrix, Graphics::viewport, camera);
	}
}

void Renderer::render_gizmo()
{
	for (Line3D& basis : gizmo.bases)
	{
		// Apply the projection matrix
		for (glm::vec4& point : basis.points)
		{
			Math3D::project(point, projection_matrix);
		}

		// Determine whether the line should be drawn or not by setting its should_render flag
		clipper.clip_line(basis);

		if (!basis.should_render)
		{
			continue;
		}

		for (glm::vec4& point : basis.points)
		{
			float one_over_w = is_nearly_zero(point.w) ? 1.0f : 1.0f / point.w;

			// Perform the perspective divide
			Math3D::to_ndc(point, one_over_w);

			// Scale into view
			Math3D::to_screen_space(point, Graphics::viewport, camera);
		}

		// Draw the gizmo line
		const glm::vec4& start = basis.points[0];
		const glm::vec4& end = basis.points[1];
		const uint32& color = basis.color;

		Graphics::draw_line_bresenham_3d(
			int(start.x), int(start.y), start.z, 
			int(end.x), int(end.y), end.z, 
			color
		);
	}
}

void Renderer::render_triangles_in_scene()
{
	for (Triangle& triangle : triangles_in_scene)
	{
		if (display_face_normals)
		{
			draw_face_normal(triangle);
		}

		// Projection
		int num_vertices = 3;
		for (int i = 0; i < num_vertices; i++)
		{
			// Transform the point from camera space to clip space
			Math3D::project(triangle.vertices[i], projection_matrix);
		}
	}

	// TODO: Clipping goes here
	BEGIN_TIMED_BLOCK(Clipping)
	std::vector<Triangle> triangles_to_rasterize = clipper.clip_triangles(triangles_in_scene);
	END_TIMED_BLOCK(Clipping)

	// Logging
	Logger::info(LOG_CATEGORY_CLIPPING, "Triangle clip coords:");
	for (const Triangle& triangle : triangles_to_rasterize)
	{
		Logger::info(LOG_CATEGORY_CLIPPING, vec4_to_string(triangle.vertices[0]));
		Logger::info(LOG_CATEGORY_CLIPPING, vec4_to_string(triangle.vertices[1]));
		Logger::info(LOG_CATEGORY_CLIPPING, vec4_to_string(triangle.vertices[2]));
	}
	Logger::info(LOG_CATEGORY_CLIPPING, "Triangle texcoords:");
	for (const Triangle& triangle : triangles_to_rasterize)
	{
		Logger::info(LOG_CATEGORY_CLIPPING, tex2_to_string(triangle.texcoords[0]));
		Logger::info(LOG_CATEGORY_CLIPPING, tex2_to_string(triangle.texcoords[1]));
		Logger::info(LOG_CATEGORY_CLIPPING, tex2_to_string(triangle.texcoords[2]));
	}

	BEGIN_TIMED_BLOCK(RenderTriangles)
	for (Triangle& triangle : triangles_to_rasterize)
	{
		// Perform conversion to NDC and viewport transform here
		int num_vertices = 3;
		for (int i = 0; i < num_vertices; i++)
		{
			// Store 1/w for later use
			triangle.inv_w[i] = is_nearly_zero(triangle.vertices[i].w) ? 1.0f : 1.0f / triangle.vertices[i].w;
			// Perform perspective divide
			Math3D::to_ndc(triangle.vertices[i], triangle.inv_w[i]);
			// Scale into view
			Math3D::to_screen_space(triangle.vertices[i], Graphics::viewport, camera);
		}

		// Perform backface culling
		if (backface_culling)
		{
			if (!triangle.is_front_facing())
			{
				continue;
			}
		}

		// Rasterization
		switch (render_mode)
		{
			case VERTICES_ONLY:
			{
				Graphics::draw_vertices(triangle, 4, Colors::YELLOW);
				break;
			}
			case WIREFRAME:
			{
				Graphics::draw_wireframe(triangle, Colors::GREEN);
				break;
			}
			case WIREFRAME_VERTICES:
			{
				Graphics::draw_wireframe(triangle, Colors::GREEN);
				Graphics::draw_vertices(triangle, 4, Colors::YELLOW);
				break;
			}
			case SOLID:
			{
				Graphics::draw_solid(triangle, Colors::WHITE);
				break;
			}
			case SOLID_WIREFRAME:
			{
				Graphics::draw_solid(triangle, Colors::WHITE);
				Graphics::draw_wireframe_3d(triangle, Colors::BLACK);
				break;
			}
			case TEXTURED:
			{
				if (!triangle.texture)
				{
					Graphics::draw_solid(triangle, Colors::RED);
				}
				else
				{
					Graphics::draw_textured(triangle);
				}
				break;
			}
			case TEXTURED_WIREFRAME:
			{
				if (!triangle.texture)
				{
					Graphics::draw_solid(triangle, Colors::RED);
					Graphics::draw_wireframe_3d(triangle, Colors::BLACK);
				}
				else
				{
					Graphics::draw_textured(triangle);
					Graphics::draw_wireframe_3d(triangle, Colors::BLACK);
				}
				break;
			}
		}
	}
	triangles_to_rasterize.clear();
	END_TIMED_BLOCK(RenderTriangles)
}

void Renderer::draw_face_normal(const Triangle& triangle)
{
	// Compute the points in 3D space to draw the lines
	float normal_length = 0.1f;
	glm::vec4 a(triangle.vertices[0]);
	glm::vec4 b(triangle.vertices[1]);
	glm::vec4 c(triangle.vertices[2]);
	glm::vec4 center = glm::vec4((a + b + c) / 3.0f);
	glm::vec4 end = center;
	end.x += (triangle.face_normal.x * normal_length);
	end.y += (triangle.face_normal.y * normal_length);
	end.z += (triangle.face_normal.z * normal_length);

	// TODO: Change to reflect the revised pipeline order
	Math3D::project_point(center, projection_matrix, Graphics::viewport, camera);
	Math3D::project_point(end, projection_matrix, Graphics::viewport, camera);

	switch (render_mode)
	{
		case VERTICES_ONLY:
		case WIREFRAME:
		case WIREFRAME_VERTICES:
			Graphics::draw_line_bresenham_3d(int(center.x), int(center.y),
				center.z, int(end.x), int(end.y), end.z, Colors::WHITE);
			break;
		case SOLID:
		case SOLID_WIREFRAME:
		case TEXTURED:
		case TEXTURED_WIREFRAME:
			Graphics::draw_line_bresenham_3d(int(center.x), int(center.y),
				center.z, int(end.x), int(end.y), end.z, Colors::GREEN);
			break;
	}
}

void Renderer::compute_face_normal(Triangle& transformed_triangle)
{
	glm::vec3 ab = glm::vec3(transformed_triangle.vertices[1] - transformed_triangle.vertices[0]);
	glm::vec3 ca = glm::vec3(transformed_triangle.vertices[2] - transformed_triangle.vertices[0]);
	glm::vec3 face_normal = glm::normalize(glm::cross(ab, ca));
	transformed_triangle.face_normal = face_normal;
}

void Renderer::set_render_mode(ERenderMode mode)
{
	render_mode = mode;
}