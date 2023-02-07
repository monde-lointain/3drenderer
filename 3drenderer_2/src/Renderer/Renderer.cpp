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
	render_mode = TEXTURED;
	display_face_normals = false;
	backface_culling = true;
	camera.position = glm::vec3(0.0f, 0.0f, -2.0f);
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
	camera.fov = 130.0f; 
	camera.aspect = 4.0f / 3.0f;
	camera.znear = 0.1f;
	camera.zfar = 50.0f;
	projection_matrix = Math3D::create_projection_matrix(camera);

	// Create the meshes in the scene
	Mesh* mesh = create_mesh("assets/models/cube.obj");
	mesh->load_texture("assets/models/checker.png");
	meshes.push_back(mesh);
}

void Renderer::process_input()
{
	BEGIN_TIMED_BLOCK(Input)
	SDL_Event event;

	/**
	 * Controls:
	 * 1-7: Toggle render mode
	 * Mouse x/y: Adjust camera pitch and yaw (TODO)
	 * WASD: Move camera in the direction of the mouse
	 * N: Toggle normals
	 * B: Toggle backface culling
	 * ESC: Quit
	 */
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
		// Initialize the model in the center of the screen
		glm::vec3 scale(1.f);
		glm::vec3 rotation(x, x, x);
		glm::vec3 translation(0.0f, 0.0f, 0.0f);
		x += 0.1f;

		// Create the world matrix for the model by concatenating rotation,
		// scaling, and transformation matrices
		glm::mat4 world_matrix =
			Math3D::create_world_matrix(scale, rotation, translation);

		// Initialize the view matrix looking in the positive z direction
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
	Graphics::clear_framebuffer(Colors::MAGENTA);
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

		// Set the values we're just transferring over
		transformed_triangle.texcoords[0] = triangle.texcoords[0];
		transformed_triangle.texcoords[1] = triangle.texcoords[1];
		transformed_triangle.texcoords[2] = triangle.texcoords[2];
		transformed_triangle.texture = triangle.texture;

		// Setup the vertices for transformation
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

		// Set the vertices on the triangle to our new transformed vertices
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

// TODO: This should really be in Math3D I think
bool Renderer::project_triangle(Triangle& triangle)
{
	int num_vertices = 3;
	for (int i = 0; i < num_vertices; i++)
	{
		// Transform the point from camera space to clip space
		Math3D::project(triangle.vertices[i], projection_matrix);
		// Store 1/w for later use
		triangle.inv_w[i] = 1.0f / triangle.vertices[i].w;
		// Perform perspective divide
		Math3D::to_ndc(triangle.vertices[i], triangle.inv_w[i]);
	}

	// Perform backface culling
	if (!triangle.is_front_facing())
	{
		return true;
	}

	for (int i = 0; i < num_vertices; i++)
	{
		// Scale into view
		Math3D::to_screen_space(triangle.vertices[i], Graphics::viewport);
	}

	return false;
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
		
		// Project the triangle into screen space. NOTE: The backface culling
		// test is performed here too!
		bool should_cull = project_triangle(triangle);

		// Check to see if the triangle should be culled
		if (backface_culling)
		{
			if (should_cull)
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
				Graphics::draw_wireframe_3d(triangle, Colors::BLACK);
				break;
			case TEXTURED:
				if (!triangle.texture)
				{
					Graphics::draw_solid(triangle, Colors::RED);
				}
				else
				{
					Graphics::draw_textured_slowly(triangle);
				}
				break;
			case TEXTURED_WIREFRAME:
				if (!triangle.texture)
				{
					Graphics::draw_solid(triangle, Colors::RED);
					Graphics::draw_wireframe_3d(triangle, Colors::BLACK);
				}
				else
				{
					Graphics::draw_textured_slowly(triangle);
					Graphics::draw_wireframe_3d(triangle, Colors::GREEN);
				}
				break;
		}
	}
	END_TIMED_BLOCK(RenderTriangles)
}

void Renderer::draw_face_normal(const Triangle& triangle)
{
	// Compute the points in 3D space to draw the lines
	float normal_length = 0.2f;
	glm::vec3 a(triangle.vertices[0].x, triangle.vertices[0].y, triangle.vertices[0].z);
	glm::vec3 b(triangle.vertices[1].x, triangle.vertices[1].y, triangle.vertices[1].z);
	glm::vec3 c(triangle.vertices[2].x, triangle.vertices[2].y, triangle.vertices[2].z);
	glm::vec4 center = glm::vec4((a + b + c) / 3.0f, 1.0f);
	glm::vec4 end = center + (glm::vec4(triangle.face_normal * normal_length, 1.0f));

	// TODO: Change to reflect the revised pipeline order
	Math3D::project_point(center, projection_matrix, Graphics::viewport);
	Math3D::project_point(end, projection_matrix, Graphics::viewport);

	switch (render_mode)
	{
		case VERTICES_ONLY:
		case WIREFRAME:
		case WIREFRAME_VERTICES:
			Graphics::draw_line_bresenham_3d(int(center.x), int(center.y),
				center.w, int(end.x), int(end.y), end.w, Colors::WHITE);
			break;
		case SOLID:
		case SOLID_WIREFRAME:
			Graphics::draw_line_bresenham_3d(int(center.x), int(center.y),
				center.w, int(end.x), int(end.y), end.w, Colors::GREEN);
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