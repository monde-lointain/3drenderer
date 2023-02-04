#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Math/Math3D.h"
#include "../Mesh/Mesh.h"
#include "../Mesh/Triangle.h"
#include "../Misc/Colors.h"
#include "../Misc/debug_counters.h"
#include "../Misc/debug_helpers.h"
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

	// Setup the camera and projection matrix
	camera.fov = 60.0f; 
	camera.aspect =
		(float)Graphics::viewport.width / (float)Graphics::viewport.height;
	camera.znear = 1.f;
	camera.zfar = 100.0f;
	projection_matrix = Math3D::create_projection_matrix(camera);

	// Create the meshes in the scene
	Mesh* mesh = create_mesh("assets/models/cube.obj");
	meshes.push_back(mesh);
}

void Renderer::process_input()
{
	BEGIN_TIMED_BLOCK(Input)
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		// Handling core SDL events (keyboard movement, closing the window etc.)
		switch (event.type)
		{
			case SDL_QUIT:
				is_running = false;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					is_running = false;
					break;
				}
				if (event.key.keysym.sym == SDLK_1)
				{
					set_render_mode(WIREFRAME_VERTICES);
					break;
				}

				if (event.key.keysym.sym == SDLK_2)
				{
					set_render_mode(WIREFRAME);
					break;
				}
				if (event.key.keysym.sym == SDLK_3)
				{
					set_render_mode(SOLID);
					break;
				}
				if (event.key.keysym.sym == SDLK_4)
				{
					set_render_mode(SOLID_WIREFRAME);
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
				break;
		}
	}
	END_TIMED_BLOCK(Input)
}

void Renderer::update()
{
	BEGIN_TIMED_BLOCK(Update)
	for (Mesh* &mesh : meshes)
	{
		// World space transformations
		glm::vec3 scale(1.0f);
		glm::vec3 rotation(x, x, x);
		glm::vec3 translation(0.0f, 0.0f, 5.0f);
		x += 0.01f;

		// Create the world matrix for the model by concatenating rotation,
		// scaling, and transformation matrices
		glm::mat4 world_matrix =
			Math3D::create_world_matrix(scale, rotation, translation);

		// Initialize the view matrix looking at the positive z direction
		glm::mat4 view_matrix = glm::lookAt(camera.position,
			glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// Update the positons of the gizmo and all triangles in 3D space
		transform_triangles(mesh, world_matrix, view_matrix);
		transform_gizmo(world_matrix, view_matrix);
	}
	END_TIMED_BLOCK(Update)
}

void Renderer::render()
{
	BEGIN_TIMED_BLOCK(Render)
	Graphics::clear_framebuffer(Colors::BLACK);
	Graphics::clear_z_buffer();

	// Render all triangles in the scene
	render_triangles_in_scene();
	// Clear the array of triangles
	triangles_in_scene.clear();

	// Project the gizmo into screen space
	project_gizmo();
	// Draw the gizmo
	Graphics::draw_gizmo(gizmo);
	// Reset the gizmo to its default position
	gizmo.reset();

	Graphics::update_framebuffer();
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

void Renderer::transform_triangles(
	Mesh* mesh, const glm::mat4& world_matrix, const glm::mat4& view_matrix)
{
	for (const Triangle& triangle : mesh->triangles)
	{
		Triangle transformed_triangle;

		glm::vec4 vertices[3] = {
			triangle.vertices[0],
			triangle.vertices[1],
			triangle.vertices[2]
		};

		// Multiply the vertices by the world and view matrices in order to get
		// them ready for projection
		for (glm::vec4& vertex : vertices)
		{
			Math3D::transform_point(vertex, world_matrix, view_matrix);
		}

		transformed_triangle.vertices[0] = vertices[0];
		transformed_triangle.vertices[1] = vertices[1];
		transformed_triangle.vertices[2] = vertices[2];

		// Compute the face normal of the triangle
		compute_face_normal(transformed_triangle);

		triangles_in_scene.push_back(transformed_triangle);
	}
}

void Renderer::transform_gizmo(
	const glm::mat4& world_matrix, const glm::mat4& view_matrix)
{
	// Transform the bases and origin
	for (glm::vec4& vector : gizmo.bases)
	{
		Math3D::transform_point(vector, world_matrix, view_matrix);
	}
	Math3D::transform_point(gizmo.origin, world_matrix, view_matrix);
}

void Renderer::project_triangle(Triangle& triangle)
{
	for (glm::vec4& vertex : triangle.vertices)
	{
		Math3D::project_point(vertex, projection_matrix, Graphics::viewport);
	}
}

void Renderer::project_gizmo()
{
	for (glm::vec4& vector : gizmo.bases)
	{
		Math3D::project_point(vector, projection_matrix, Graphics::viewport);
	}
	Math3D::project_point(gizmo.origin, projection_matrix, Graphics::viewport);
}

void Renderer::render_triangles_in_scene()
{
	BEGIN_TIMED_BLOCK(RenderTriangles)
	for (Triangle triangle : triangles_in_scene)
	{
		if (display_face_normals)
		{
			draw_face_normal(triangle);
		}
		
		// Project the triangle into screen space
		project_triangle(triangle);

		// Check to see if the triangle should be culled
		if (backface_culling)
		{
			if (triangle.is_facing_camera())
			{
				continue;
			}
		}

		// Render
		switch (render_mode)
		{
			case VERTICES_ONLY:
				Graphics::draw_vertices(triangle, 4, Colors::YELLOW);
				break;
			case WIREFRAME:
				Graphics::draw_wireframe(triangle, Colors::GREEN);
				break;
			case WIREFRAME_VERTICES:
				Graphics::draw_wireframe(triangle, Colors::GREEN);
				Graphics::draw_vertices(triangle, 4, Colors::YELLOW);
				break;
			case SOLID:
				Graphics::draw_solid(triangle, Colors::WHITE);
				break;
			case SOLID_WIREFRAME:
				Graphics::draw_solid(triangle, Colors::WHITE);
				Graphics::draw_wireframe(triangle, Colors::BLACK);
				break;
		}
	}
	END_TIMED_BLOCK(RenderTriangles)
}

void Renderer::draw_face_normal(const Triangle& triangle)
{
	// Compute the points in 3D space to draw the lines
	float normal_length = 1.f;
	glm::vec3 a(triangle.vertices[0].x, triangle.vertices[0].y, triangle.vertices[0].z);
	glm::vec3 b(triangle.vertices[1].x, triangle.vertices[1].y, triangle.vertices[1].z);
	glm::vec3 c(triangle.vertices[2].x, triangle.vertices[2].y, triangle.vertices[2].z);
	glm::vec4 center = glm::vec4((a + b + c) / 3.0f, 1.0f);
	glm::vec4 end = center + (glm::vec4(triangle.face_normal, 1.0f) * normal_length);

	Math3D::project_point(center, projection_matrix, Graphics::viewport);
	Math3D::project_point(end, projection_matrix, Graphics::viewport);

	// Draw a line from the center to the center + normal
	Graphics::draw_line_bresenham((int)center.x, (int)center.y, (int)end.x, (int)end.y, Colors::WHITE);
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