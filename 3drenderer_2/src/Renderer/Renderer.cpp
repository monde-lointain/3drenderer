#include "Renderer.h"

#include "Viewport.h"
#include "../Math/Math3D.h"
#include "../Mesh/Triangle.h"
#include "../Utils/Colors.h"
#include "../Utils/math_helpers.h"
#include "../Window/Window.h"
#include "../World/World.h"
#include <SDL.h>
#include <tracy/tracy/Tracy.hpp>

constexpr int MAX_TRIANGLES = 100000;

void Renderer::initialize(std::shared_ptr<Window> app_window,
	std::shared_ptr<Viewport> app_viewport,
	std::shared_ptr<World> app_world)
{
	// Assign the window, viewport and world pointers
	viewport = app_viewport;
	window = app_window;
	world = app_world;

	graphics = std::make_unique<Graphics>();
	graphics->init(window->renderer, viewport);
	graphics->initialize_framebuffer();

	render_mode = SOLID;
	shading_mode = GOURAUD;
	display_face_normals = false;
	backface_culling = true;

	// Create the array of triangles that will be rasterized
	triangles_to_rasterize = std::make_unique<Triangle[]>(MAX_TRIANGLES);
}

void Renderer::destroy()
{
	graphics->free_framebuffer();
}

void Renderer::render()
{
	ZoneScoped; // for tracy

	graphics->clear_framebuffer(Colors::BLACK);
	graphics->clear_z_buffer();

	// Render all triangles in the scene
	render_triangles_in_scene();
	// Clear the array of triangles
	world->triangles_in_scene.clear();

	// Render all lines in the scene
	render_lines();
	// Reset the positions of the gizmo
	world->gizmo.reset();

	graphics->update_framebuffer();
}

void Renderer::render_triangles_in_scene()
{
	ZoneScoped; // for tracy

	for (Triangle& triangle : world->triangles_in_scene)
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
			Math3D::project(triangle.vertices[i], world->camera.projection_matrix);
		}
	}

	ZoneNamedN(clip_fill_scope, "Clipping and filling", true); // for tracy

	int num_triangles_to_rasterize = 0;
	// Clip all the triangles and stick them in a new array
	clipper.clip_triangles(
		world->triangles_in_scene, triangles_to_rasterize, num_triangles_to_rasterize);

	ZoneNamedN(rasterize_triangles_scope, "Rasterization", true); // for tracy

#pragma omp parallel
#pragma omp for schedule(dynamic,10) // give each thread ten triangles at a time
	for (int i = 0; i < num_triangles_to_rasterize; i++)
	{
		ZoneNamedN(render_triangle_scope, "Render triangle", true); // for tracy

		Triangle& triangle = triangles_to_rasterize[i];

		// Perform conversion to NDC and viewport transform here
		int num_vertices = 3;
		for (int j = 0; j < num_vertices; j++)
		{
			// Store 1/w for later use
			triangle.inv_w[j] = is_nearly_zero(triangle.vertices[j].w)
									? 1.0f
									: 1.0f / triangle.vertices[j].w;
			// Perform perspective divide
			Math3D::to_ndc(triangle.vertices[j], triangle.inv_w[j]);
			// Scale into view
			Math3D::to_screen_space(
				triangle.vertices[j], viewport, world->camera);
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
				graphics->draw_vertices(triangle, 4, Colors::YELLOW);
				break;
			}
			case WIREFRAME:
			{
				graphics->draw_wireframe(triangle, Colors::GREEN);
				break;
			}
			case WIREFRAME_VERTICES:
			{
				graphics->draw_wireframe(triangle, Colors::GREEN);
				graphics->draw_vertices(triangle, 4, Colors::YELLOW);
				break;
			}
			case SOLID:
			{
				graphics->draw_solid(triangle, Colors::WHITE, shading_mode);
				break;
			}
			case SOLID_WIREFRAME:
			{
				graphics->draw_solid(triangle, Colors::WHITE, shading_mode);
				graphics->draw_wireframe_3d(triangle, Colors::BLACK);
				break;
			}
			case TEXTURED:
			{
				if (!triangle.texture)
				{
					graphics->draw_solid(triangle, Colors::RED, NONE);
				}
				else
				{
					graphics->draw_textured(triangle, shading_mode);
				}
				break;
			}
			case TEXTURED_WIREFRAME:
			{
				if (!triangle.texture)
				{
					graphics->draw_solid(triangle, Colors::RED, NONE);
					graphics->draw_wireframe_3d(triangle, Colors::BLACK);
				}
				else
				{
					graphics->draw_textured(triangle, shading_mode);
					graphics->draw_wireframe_3d(triangle, Colors::BLACK);
				}
				break;
			}
		}
	}
}

void Renderer::render_lines()
{

	// Render each line
	for (Line3D& line : world->lines_in_scene)
	{
		// Apply the projection matrix
		for (glm::vec4& point : line.points)
		{
			Math3D::project(point, world->camera.projection_matrix);
		}

		// Determine whether the line should be drawn or not by setting its should_render flag
		clipper.clip_line(line);

		if (!line.should_render)
		{
			continue;
		}

		for (glm::vec4& point : line.points)
		{
			float one_over_w = is_nearly_zero(point.w) ? 1.0f : 1.0f / point.w;

			// Perform the perspective divide
			Math3D::to_ndc(point, one_over_w);

			// Scale into view
			Math3D::to_screen_space(point, viewport, world->camera);
		}

		// Draw the line
		const glm::vec4& start = line.points[0];
		const glm::vec4& end = line.points[1];
		const uint32& color = line.color;

		graphics->draw_line_bresenham_3d(
			int(start.x), int(start.y), start.z,
			int(end.x), int(end.y), end.z,
			color
		);
	}
	world->lines_in_scene.clear();
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
	Math3D::project_point(center, world->camera.projection_matrix, viewport, world->camera);
	Math3D::project_point(end, world->camera.projection_matrix, viewport, world->camera);

	switch (render_mode)
	{
	case VERTICES_ONLY:
	case WIREFRAME:
	case WIREFRAME_VERTICES:
		graphics->draw_line_bresenham_3d(int(center.x), int(center.y),
			center.z, int(end.x), int(end.y), end.z, Colors::WHITE);
		break;
	case SOLID:
	case SOLID_WIREFRAME:
	case TEXTURED:
	case TEXTURED_WIREFRAME:
		graphics->draw_line_bresenham_3d(int(center.x), int(center.y),
			center.z, int(end.x), int(end.y), end.z, Colors::GREEN);
		break;
	}
}

void Renderer::display_frame()
{
	graphics->render_frame();
}

void Renderer::set_render_mode(ERenderMode mode)
{
	render_mode = mode;
}

void Renderer::set_shading_mode(EShadingMode mode)
{
	// Toggle off if the user presses the same button twice in a row
	if (shading_mode == mode)
	{
		shading_mode = NONE;
	}
	else
	{
		shading_mode = mode;
	}
}
