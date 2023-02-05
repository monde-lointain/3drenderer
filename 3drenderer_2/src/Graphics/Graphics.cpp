#include "Graphics.h"

#include "../Math/3d_vector.h"
#include "../Math/Math3D.h"
#include "../Mesh/Triangle.h"
#include "../Misc/Colors.h"
#include "../Misc/debug_helpers.h"
#include "../Renderer/Gizmo.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <SDL.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

Viewport Graphics::viewport = { 1280, 720, "3D Renderer" };

static uint32* framebuffer = nullptr;
static SDL_Texture* framebuffer_texture = nullptr;

static float* depth_buffer = nullptr;

bool Graphics::initialize_window()
{
	viewport = { 1280, 720, "3D Renderer" };

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			viewport.name.c_str(),
			"Error initializing SDL.",
			nullptr);
		return false;
	}

	window = SDL_CreateWindow(viewport.name.c_str(),
		                      SDL_WINDOWPOS_CENTERED,
		                      SDL_WINDOWPOS_CENTERED,
		                      viewport.width,
		                      viewport.height,
		                      SDL_WINDOW_BORDERLESS);

	debug_heap_check();

	if (!window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			                     viewport.name.c_str(),
			                     "Error creating SDL window.",
			                     nullptr);
		return false;
	}

	renderer = SDL_CreateRenderer(window,
		                          -1,
		                          SDL_RENDERER_ACCELERATED |
		                          SDL_RENDERER_PRESENTVSYNC |
		                          SDL_RENDERER_TARGETTEXTURE);

	debug_heap_check();

	if (!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			                     viewport.name.c_str(),
			                     "Error creating SDL renderer.",
			                     nullptr);
		return false;
	}


	debug_heap_check();

	return true;
}

void Graphics::initialize_framebuffer()
{
	framebuffer = new uint32[viewport.width * viewport.height];
	assert(framebuffer);

	framebuffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_BGRA8888,
		SDL_TEXTUREACCESS_STREAMING,
		viewport.width,
		viewport.height
	);
	assert(framebuffer_texture);

	depth_buffer = new float[viewport.width * viewport.height];
	assert(framebuffer);
}

void Graphics::clear_framebuffer(uint32 color)
{
	int i;
	int size = viewport.width * viewport.height;
	int loop_count = size / 8;

	// Set up a register with 8 instances of the same color value
	__m256i color_vec = _mm256_set1_epi32(color);

	// Eight pixels at a time until we have less than eight remaining
	for (i = 0; i < loop_count * 8; i += 8)
	{
		_mm256_store_si256((__m256i*)(framebuffer + i), color_vec);
	}

	// Clear the remaining values in the buffer
	for (; i < size; i++)
	{
		framebuffer[i] = color;
	}
}

void Graphics::clear_z_buffer()
{
	int i;
	int size = viewport.width * viewport.height;
	int loop_count = size / 8;

	const float MAX = std::numeric_limits<float>::max();
	// Set up a register with 8 float values set to the max possible value for a
	// float. We'll only render pixels if they are in front (less) of this value
	__m256 max = _mm256_set1_ps(MAX);

	// Eight pixels at a time until we have less than eight remaining
	for (i = 0; i < loop_count * 8; i += 8)
	{
		_mm256_store_ps(&depth_buffer[i], max);
	}

	// Clear the remaining values in the buffer
	for (; i < size; i++)
	{
		depth_buffer[i] = MAX;
	}
}

void Graphics::update_framebuffer()
{
	SDL_UpdateTexture(framebuffer_texture,
		nullptr,
		framebuffer,
		viewport.width * sizeof(uint32));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer_texture, nullptr, nullptr);
}

void Graphics::render_frame()
{
	SDL_RenderPresent(renderer);
}

void Graphics::draw_pixel(const int& x, const int& y, const uint32& color)
{
	// Clip any pixels that are drawn outside the viewport
	if (!is_in_viewport(x, y))
	{
		return;
	}
	// Draw the framebuffer starting from the bottom left corner of the screen.
	int index = viewport.width * (viewport.height - y - 1) + x;
	framebuffer[index] = color;
}

void Graphics::draw_rect(const int& x, const int& y, const int& width,
	const int& height, const uint32& color)
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int current_x = x + i;
			int current_y = y + j;
			draw_pixel(current_x, current_y, color);
		}
	}
}

void Graphics::draw_line_dda(const int& x0, const int& y0, const int& x1,
	const int& y1, const uint32& color)
{
	int dx = (x1 - x0);
	int dy = (y1 - y0);

	int longest_side_length = abs(dx) >= abs(dy) ? abs(dx) : abs(dy);

	float x_inc = dx / (float)longest_side_length;
	float y_inc = dy / (float)longest_side_length;

	float current_x = (float)x0;
	float current_y = (float)y0;

	for (int i = 0; i <= longest_side_length; i++)
	{
		draw_pixel((int)current_x, (int)current_y, color);
		current_x += x_inc;
		current_y += y_inc;
	}
}

void Graphics::draw_line_bresenham(const int& x0, const int& y0, const int& x1,
	const int& y1, const uint32& color)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	// Determine the step direction for x and y
	int x_inc = (x0 < x1) ? 1 : -1;
	int y_inc = (y0 < y1) ? 1 : -1;

	int error = dx - dy;
	int e2 = 2 * error;

	int current_x = x0;
	int current_y = y0;

	// Loop until the line is drawn
	while (true)
	{
		draw_pixel(current_x, current_y, color);

		// Stop once both endpoints have been drawn
		if (current_x == x1 && current_y == y1)
		{
			break;
		}

		e2 = 2 * error;
		// Update the error value
		if (e2 > -dy)
		{
			current_x += x_inc;
			error -= dy;
		}
		if (e2 < dx)
		{
			current_y += y_inc;
			error += dx;
		}
	}
}

void Graphics::draw_line_bresenham_3d(const int& x1, const int& y1,
	const float& w1, const int& x2, const int& y2, const float& w2,
	const uint32& color)
{
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	// Determine the step direction for x and y
	int x_inc = x1 < x2 ? 1 : -1;
	int y_inc = y1 < y2 ? 1 : -1;

	int error = dx - dy;
	int e2 = 2 * error;

	int current_x = x1;
	int current_y = y1;
	float depth = w1;

	// Loop until the line is drawn
	while (true)
	{
		// Calculate the index into the z-buffer for this pixel
		int index = viewport.width * (viewport.height - current_y - 1) + current_x;

		// Check if the pixel is within the bounds of the screen
		if (current_x >= 0 && current_x < viewport.width && 
			current_y >= 0 && current_y < viewport.height)
		{
			// Interpolate the depth of the pixel based on the depth of the two
			// endpoints
			if (dx > dy)
			{
				depth = w1 + (w2 - w1) * (float)(current_x - x1) / (float)(x2 - x1);
			}
			else
			{
				depth = w1 + (w2 - w1) * (float)(current_y - y1) / (float)(y2 - y1);
			}

			// Perform a depth check using the z-buffer
			if (depth <= depth_buffer[index])
			{
				// Render the pixel
				depth_buffer[index] = depth;
				draw_pixel(current_x, current_y, color);
			}
		}

		// Stop once both endpoints have been drawn
		if (current_x == x2 && current_y == y2)
		{
			break;
		}

		e2 = 2 * error;
		// Update the error value
		if (e2 > -dy)
		{
			current_x += x_inc;
			error -= dy;
		}
		if (e2 < dx)
		{
			current_y += y_inc;
			error += dx;
		}
	}
}

void Graphics::draw_wireframe(const Triangle& triangle, const uint32& color)
{
	vec2i a = { int(triangle.vertices[0].x), int(triangle.vertices[0].y) };
	vec2i b = { int(triangle.vertices[1].x), int(triangle.vertices[1].y) };
	vec2i c = { int(triangle.vertices[2].x), int(triangle.vertices[2].y) };
	draw_line_bresenham(a.x, a.y, b.x, b.y, color);
	draw_line_bresenham(b.x, b.y, c.x, c.y, color);
	draw_line_bresenham(c.x, c.y, a.x, a.y, color);
}

void Graphics::draw_wireframe_3d(const Triangle& triangle, const uint32& color)
{
	vec2i a = { int(triangle.vertices[0].x), int(triangle.vertices[0].y) };
	vec2i b = { int(triangle.vertices[1].x), int(triangle.vertices[1].y) };
	vec2i c = { int(triangle.vertices[2].x), int(triangle.vertices[2].y) };
	float wa = triangle.vertices[0].w;
	float wb = triangle.vertices[1].w;
	float wc = triangle.vertices[2].w;
	draw_line_bresenham_3d(a.x, a.y, wa, b.x, b.y, wb, color);
	draw_line_bresenham_3d(b.x, b.y, wb, c.x, c.y, wc, color);
	draw_line_bresenham_3d(c.x, c.y, wc, a.x, a.y, wa, color);
}

void Graphics::draw_solid(const Triangle& triangle, const uint32& color)
{
	vec2i v0 = { int(triangle.vertices[0].x), int(triangle.vertices[0].y) };
	vec2i v1 = { int(triangle.vertices[1].x), int(triangle.vertices[1].y) };
	vec2i v2 = { int(triangle.vertices[2].x), int(triangle.vertices[2].y) };
	float v0w = triangle.vertices[0].w;
	float v1w = triangle.vertices[1].w;
	float v2w = triangle.vertices[2].w;
	tex2 uv0 = { triangle.texcoords[0].u,triangle.texcoords[0].v };
	tex2 uv1 = { triangle.texcoords[1].u,triangle.texcoords[1].v };
	tex2 uv2 = { triangle.texcoords[2].u,triangle.texcoords[2].v };

	// Calculate triangle bounding box
	int min_x = std::min({ int(v0.x), int(v1.x), int(v2.x) });
	int max_x = std::max({ int(v0.x), int(v1.x), int(v2.x) });
	int min_y = std::min({ int(v0.y), int(v1.y), int(v2.y) });
	int max_y = std::max({ int(v0.y), int(v1.y), int(v2.y) });

	// Clip triangle bounding box to screen
	min_x = std::max(min_x, 0);
	max_x = std::min(max_x, viewport.width - 1);
	min_y = std::max(min_y, 0);
	max_y = std::min(max_y, viewport.height - 1);
	
	// Compute the inverse area of the triangle
	float area = (float)Math3D::orient2d_i(v0, v1, v2);
	float inv_area = 1.0f / area;

	vec2i p;
	// Loop over the bounding box and rasterize the triangle
	for (p.y = min_y; p.y <= max_y; p.y++) {
		for (p.x = min_x; p.x <= max_x; p.x++) {
			// Compute barycentric weights
			float alpha = (float)Math3D::orient2d_i(v1, v2, p);
			float beta = (float)Math3D::orient2d_i(v2, v0, p);
			float gamma = (float)Math3D::orient2d_i(v0, v1, p);
			alpha *= inv_area;
			beta *= inv_area;
			gamma *= inv_area;

			// Check if the point is inside the triangle
			if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f)
			{
				// Interpolate depth value
				float depth = v0w * (float)alpha + v1w * (float)beta + v2w * (float)gamma;

				// Check depth against z-buffer and only render if the pixel is in front
				int index = viewport.width * (viewport.height - p.y - 1) + p.x;
				if (depth >= depth_buffer[index]) {
					continue;
				}
				depth_buffer[index] = depth;

				// Interpolate texture coordinates
				float u = uv0.u * alpha + uv1.u * beta + uv2.u * gamma;
				float v = uv0.v * alpha + uv1.v * beta + uv2.v * gamma;

				// Perform perspective correct texture mapping
				u /= depth;
				v /= depth;

				// Look up texel value and set pixel color
				draw_pixel(p.x, p.y, color);
			}
		}
	}
}

void Graphics::draw_vertices(
	const Triangle& triangle, int point_size, const uint32& color)
{
	int offset = point_size / 2;
	for (glm::vec4 vertex : triangle.vertices)
	{
		int x_pos = (int)vertex.x - offset;
		int y_pos = (int)vertex.y - offset;
		Graphics::draw_rect(x_pos, y_pos, point_size, point_size, color);
	}
}

void Graphics::draw_gizmo(const Gizmo& gizmo)
{
	// x axis
	draw_line_bresenham(
		int(gizmo.origin.x), int(gizmo.origin.y),
		int(gizmo.bases[0].x), int(gizmo.bases[0].y),
		Colors::YELLOW
	);
	// y axis
	draw_line_bresenham(
		int(gizmo.origin.x), int(gizmo.origin.y),
		int(gizmo.bases[1].x), int(gizmo.bases[1].y),
		Colors::MAGENTA
	);
	// z axis
	draw_line_bresenham(
		int(gizmo.origin.x), int(gizmo.origin.y),
		int(gizmo.bases[2].x), int(gizmo.bases[2].y),
		Colors::CYAN
	);
}

void Graphics::close_window()
{
	// Free the resources allocated
	delete[] framebuffer;
	delete[] depth_buffer;
	SDL_DestroyTexture(framebuffer_texture);
	// Close SDL
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

bool Graphics::is_in_viewport(int x, int y)
{
	bool x_in_viewport = x >= 0 && x < viewport.width;
	bool y_in_viewport = y >= 0 && y < viewport.height;
	return x_in_viewport && y_in_viewport;
}

uint32 Graphics::get_zbuffer_color(float val)
{
	// Convert to 8 bits (0-255)
	uint8 color = (uint8)(val * 255.0f + 0.5f);
	uint8 alpha = 0xFF;
	// Return the 32-bit BGRA value
	uint32 result = (color << 24) | (color << 16) | (color << 8) | alpha;
	return result;
}