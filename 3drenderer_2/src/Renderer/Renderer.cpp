#include "Renderer.h"

#include <thread>

#include <omp.h>
#include <tracy/tracy/Tracy.hpp>

#include "../Clipping/Clipper.h"
#include "../Graphics/Graphics.h"
#include "../Math/Math3D.h"
#include "../Triangle/Triangle.h"
#include "../Utils/Colors.h"
#include "../Utils/math_helpers.h"
#include "../Viewport/Viewport.h"
#include "../Window/Window.h"
#include "../World/World.h"

#ifdef _MSC_VER // Windows
#include <SDL.h>
#else // Linux
#include <SDL2/SDL.h>
#endif


void Renderer::initialize(
	std::shared_ptr<Window> app_window,
	std::shared_ptr<Viewport> app_viewport,
	std::shared_ptr<World> app_world
)
{
	// Assign the window, viewport and world pointers
	viewport = std::move(app_viewport);
	window = std::move(app_window);
	world = std::move(app_world);

	graphics_init(window->renderer, viewport);
	initialize_framebuffer();

	render_mode = TEXTURED_WIREFRAME;
	shading_mode = GOURAUD;
	display_face_normals = false;
	backface_culling = true;

	// Create the array of triangles that will be rasterized
	triangles_to_rasterize = std::make_unique<std::array<Triangle, MAX_TRIANGLES>>();
}

void Renderer::destroy()
{
	free_framebuffer();
}

void Renderer::render()
{
	ZoneScoped; // for tracy

	clear_framebuffer(Colors::BLACK);
	clear_z_buffer();

	// Render all triangles in the scene
	render_triangles_in_scene();
	// Clear the array of triangles
	world->triangles_in_scene.clear();

	// Render all lines in the scene
	render_lines();
	// Reset the positions of the gizmo
	world->gizmo.reset();

	update_framebuffer();
}

constexpr int NUM_TRIANGLES_PER_BATCH = 10;

void Renderer::render_triangles_in_scene()
{
	ZoneScoped; // for tracy

	for (Triangle& triangle : world->triangles_in_scene)
	{
		if (display_face_normals)
		{
			draw_face_normal(triangle);
		}

		for (Vertex& vertex : triangle.vertices)
		{
			// Transform the point from camera space to clip space
			Math3D::project(vertex.position, world->camera.projection_matrix);
		}
	}

	ZoneNamedN(clip_fill_scope, "Clipping and filling", true); // for tracy

	int num_triangles_to_rasterize = 0;
	// Clip all the triangles and stick them in a new array
	clip_triangles(
		world->triangles_in_scene, 
		triangles_to_rasterize->data(),
		num_triangles_to_rasterize
	);

	ZoneNamedN(rasterize_triangles_scope, "Rasterization", true); // for tracy

//// Set the number of threads to execute to the total number of physical cores on
//// the machine, plus one. Note that this function assumes that the PC has two
//// logical cores per physical core
//#pragma omp parallel \
//	num_threads((std::thread::hardware_concurrency() / 2) + 1) \
//	default(none) \
//	shared(num_triangles_to_rasterize)
//// Give each thread ten triangles at a time
//#pragma omp for schedule(static, NUM_TRIANGLES_PER_BATCH)
	for (int i = 0; i < num_triangles_to_rasterize; i++)
	{
		ZoneNamedN(render_triangle_scope, "Render triangle", true); // for tracy

		Triangle& triangle = triangles_to_rasterize->data()[i];

		// Perform conversion to NDC and viewport transform here
		for (Vertex& vertex : triangle.vertices)
		{
			// Store 1/w for later use
			vertex.position.w = is_nearly_zero(vertex.position.w)
									? 1.0f
									: 1.0f / vertex.position.w;
			// Perform perspective divide
			Math3D::to_ndc(vertex.position, vertex.position.w);
			// Scale into view
			Math3D::to_screen_space(vertex.position, viewport, world->camera);
		}

		// Perform backface culling
		if (backface_culling)
		{
			if (!triangle.is_front_facing())
			{
				continue;
			}
		}

		rasterize_triangle(triangle);
	}
}

void Renderer::render_lines() const
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
		clip_line(line);

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
		const glm::ivec2 start(lrintf(line.points[0].x), lrintf(line.points[0].y));
		const glm::ivec2 end(lrintf(line.points[1].x), lrintf(line.points[1].y));
		const float start_z = line.points[0].z;
		const float end_z = line.points[1].z;
		const uint32 color = line.color;

		draw_line_bresenham_3d(start, end, start_z, end_z, color);
	}
	world->lines_in_scene.clear();
}

void Renderer::rasterize_triangle(Triangle& triangle) const
{
	// Rasterization
	switch (render_mode)
	{
		case VERTICES_ONLY:
		{
			draw_vertices(triangle, 4, Colors::YELLOW);
			break;
		}
		case WIREFRAME:
		{
			draw_wireframe(triangle, Colors::GREEN);
			break;
		}
		case WIREFRAME_VERTICES:
		{
			draw_wireframe(triangle, Colors::GREEN);
			draw_vertices(triangle, 4, Colors::YELLOW);
			break;
		}
		case SOLID:
		{
			draw_solid(triangle, Colors::WHITE, shading_mode);
			break;
		}
		case SOLID_WIREFRAME:
		{
			draw_solid(triangle, Colors::WHITE, shading_mode);
			draw_wireframe_3d(triangle, Colors::BLACK);
			break;
		}
		case TEXTURED:
		{
			if (!triangle.texture)
			{
				draw_solid(triangle, Colors::RED, NONE);
			}
			else
			{
				draw_textured(triangle, shading_mode);
			}
			break;
		}
		case TEXTURED_WIREFRAME:
		{
			if (!triangle.texture)
			{
				draw_solid(triangle, Colors::RED, NONE);
				draw_wireframe_3d(triangle, Colors::BLACK);
			}
			else
			{
				draw_textured(triangle, shading_mode);
				draw_wireframe_3d(triangle, Colors::BLACK);
			}
			break;
		}
	}
}

/* This function is completely broken due to the  No idea if I'll fix it or not */
void Renderer::draw_face_normal(const Triangle& triangle) const
{
	// Compute the points in 3D space to draw the lines
	const float normal_length = 0.1f;
	const glm::vec4 a(triangle.vertices[0].position);
	const glm::vec4 b(triangle.vertices[1].position);
	const glm::vec4 c(triangle.vertices[2].position);
	glm::vec4 center = glm::vec4((a + b + c) / 3.0f);
	glm::vec4 end = center;
	end.x += (triangle.face_normal.x * normal_length);
	end.y += (triangle.face_normal.y * normal_length);
	end.z += (triangle.face_normal.z * normal_length);

	Math3D::project_point(center, world->camera.projection_matrix, viewport, world->camera);
	Math3D::project_point(end, world->camera.projection_matrix, viewport, world->camera);

	const glm::ivec2 center_(lrintf(center.x), lrintf(center.y));
	const glm::ivec2 end_(lrintf(end.x), lrintf(end.y));
	const float center_z = center.z;
	const float end_z = end.z;

	switch (render_mode)
	{
	case VERTICES_ONLY:
	case WIREFRAME:
	case WIREFRAME_VERTICES:
		draw_line_bresenham_3d(center_, end_, center_z, end_z, Colors::WHITE);
		break;
	case SOLID:
	case SOLID_WIREFRAME:
	case TEXTURED:
	case TEXTURED_WIREFRAME:
		draw_line_bresenham_3d(center_, end_, center_z, end_z, Colors::GREEN);
		break;
	}
}

void Renderer::display_frame()
{
	render_frame();
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
