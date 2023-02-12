#include "Graphics.h"

#include "GUI.h"
#include "../Math/3d_vector.h"
#include "../Math/Math3D.h"
#include "../Mesh/Texture.h"
#include "../Mesh/Triangle.h"
#include "../Utils/Colors.h"
#include "../Utils/debug_helpers.h"
#include "../Renderer/Gizmo.h"
#include <glm/vec2.hpp>
#include <SDL.h>
#include <tracy/tracy/Tracy.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <memory>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

Viewport Graphics::viewport = { 1920, 1080, "3D Renderer" };

std::unique_ptr<GUI> gui = std::make_unique<GUI>();

static uint32* framebuffer = nullptr;
static SDL_Texture* framebuffer_texture = nullptr;

static float* depth_buffer = nullptr;

int window_x;
int window_y;
int mouse_x;
int mouse_y;
bool is_dragging = false;

bool Graphics::initialize_window()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			viewport.name.c_str(),
			"Error initializing SDL.",
			nullptr);
		return false;
	}

	// Create the window
	window = SDL_CreateWindow(viewport.name.c_str(),
		                      SDL_WINDOWPOS_CENTERED,
		                      SDL_WINDOWPOS_CENTERED,
		                      viewport.width,
		                      viewport.height,
		                      SDL_WINDOW_BORDERLESS);

	if (!window)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			                     viewport.name.c_str(),
			                     "Error creating SDL window.",
			                     nullptr);
		return false;
	}

	// Create the SDL renderer
	renderer = SDL_CreateRenderer(window,
		                          -1,
		                          SDL_RENDERER_ACCELERATED |
		                          SDL_RENDERER_PRESENTVSYNC |
		                          SDL_RENDERER_TARGETTEXTURE);

	if (!renderer)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			                     viewport.name.c_str(),
			                     "Error creating SDL renderer.",
			                     nullptr);
		return false;
	}

	// Create the GUI
	if (!gui->init(window, renderer))
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
			                     viewport.name.c_str(),
			                     "Error initializing ImGui with SDL renderer.", 
			                     nullptr);
		return false;
	}

	return true;
}

void Graphics::window_clicked(SDL_Event& event)
{
	//SDL_SetRelativeMouseMode(SDL_TRUE);
	//SDL_GetWindowPosition(window, &window_x, &window_y);
	//mouse_x = event.button.x;
	//mouse_y = event.button.y;
	//is_dragging = true;
}

void Graphics::window_released()
{
	//SDL_SetRelativeMouseMode(SDL_FALSE);
	//is_dragging = false;
}

void Graphics::drag_window(SDL_Event& event)
{
	//if (is_dragging) {
	//	int new_x = window_x + event.motion.x - mouse_x;
	//	int new_y = window_y + event.motion.y - mouse_y;
	//	SDL_SetWindowPosition(window, new_x, new_y);
	//}
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

void Graphics::clear_framebuffer(const uint32& color)
{
	ZoneScoped; // for tracy

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
	ZoneScoped; // for tracy

	int i;
	int size = viewport.width * viewport.height;
	int loop_count = size / 8;

	constexpr float MAX = std::numeric_limits<float>::max();
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
	ZoneScoped; // for tracy

	SDL_UpdateTexture(framebuffer_texture,
		nullptr,
		framebuffer,
		viewport.width * sizeof(uint32));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer_texture, nullptr, nullptr);
}

void Graphics::gui_process_input(SDL_Event& event)
{
	gui->process_input(event);
}

void Graphics::render_gui()
{
	gui->render();
}

void Graphics::render_frame()
{
	ZoneScoped; // for tracy

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
	ZoneScoped; // for tracy

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
		ZoneScoped;

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
	const float& z1, const int& x2, const int& y2, const float& z2,
	const uint32& color)
{
	ZoneScoped; // for tracy

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	// Determine the step direction for x and y
	int x_inc = x1 < x2 ? 1 : -1;
	int y_inc = y1 < y2 ? 1 : -1;

	int error = dx - dy;
	int e2 = 2 * error;

	int current_x = x1;
	int current_y = y1;
	float depth = z1;

	// For depth interpolation
	float dz = z2 - z1;
	float line_len =
		sqrtf((float)((x2 - x1) * (x2 - x1)) + (float)((y2 - y1) * (y2 - y1)));

	// Loop until the line is drawn
	while (true)
	{
		// Calculate the index into the z-buffer for this pixel
		int index = viewport.width * (viewport.height - current_y - 1) + current_x;

		// Check if the pixel is within the bounds of the screen
		if (current_x >= 0 && current_x < viewport.width && 
			current_y >= 0 && current_y < viewport.height)
		{
			// Is there a faster way to do this?
			float curr_len =
				sqrtf((float)((current_x - x1) * (current_x - x1))
					  + (float)((current_y - y1) * (current_y - y1)));
			float pct = curr_len / line_len;
			depth = z1 + (dz * pct);

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

void Graphics::draw_line_bresenham_3d_no_zfight(const int& x1, const int& y1,
	const float& z1, const glm::vec3& n1, const int& x2, const int& y2,
	const float& z2, const glm::vec3& n2, const uint32& color)
{
	ZoneScoped; // for tracy

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	// Determine the step direction for x and y
	int x_inc = x1 < x2 ? 1 : -1;
	int y_inc = y1 < y2 ? 1 : -1;

	int error = dx - dy;
	int e2 = 2 * error;

	int current_x = x1;
	int current_y = y1;
	float depth = z1;
	glm::vec3 normal = n1;

	// For depth/normal interpolation
	float line_len =
		sqrtf((float)((x2 - x1) * (x2 - x1)) + (float)((y2 - y1) * (y2 - y1)));
	float dz = z2 - z1;
	glm::vec3 dn = n2 - n1;

	// Loop until the line is drawn
	while (true)
	{
		// Calculate the index into the z-buffer for this pixel
		int index = viewport.width * (viewport.height - current_y - 1) + current_x;

		// Check if the pixel is within the bounds of the screen
		if (current_x >= 0 && current_x < viewport.width &&
			current_y >= 0 && current_y < viewport.height)
		{
			// Is there a faster way to do this?
			float curr_len =
				sqrtf((float)((current_x - x1) * (current_x - x1))
					+ (float)((current_y - y1) * (current_y - y1)));
			float pct = curr_len / line_len;
			depth = z1 + (dz * pct);

			// Interpolate the normals
			normal = n1 + (dn * pct);

			// Push the z value away from the mesh a little bit in the opposite
			// direction of its transformed normal vector
			float offset_factor = 0.00001f;
			depth -= (normal.z * offset_factor);

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
	ZoneScoped; // for tracy

	vec2i a = { int(triangle.vertices[0].x), int(triangle.vertices[0].y) };
	vec2i b = { int(triangle.vertices[1].x), int(triangle.vertices[1].y) };
	vec2i c = { int(triangle.vertices[2].x), int(triangle.vertices[2].y) };
	draw_line_bresenham(a.x, a.y, b.x, b.y, color);
	draw_line_bresenham(b.x, b.y, c.x, c.y, color);
	draw_line_bresenham(c.x, c.y, a.x, a.y, color);
}

void Graphics::draw_wireframe_3d(const Triangle& triangle, const uint32& color)
{
	ZoneScoped; // for tracy

	vec2i a = { int(triangle.vertices[0].x), int(triangle.vertices[0].y) };
	vec2i b = { int(triangle.vertices[1].x), int(triangle.vertices[1].y) };
	vec2i c = { int(triangle.vertices[2].x), int(triangle.vertices[2].y) };
	float za = triangle.vertices[0].z;
	float zb = triangle.vertices[1].z;
	float zc = triangle.vertices[2].z;
	glm::vec3 na = triangle.normals[0];
	glm::vec3 nb = triangle.normals[1];
	glm::vec3 nc = triangle.normals[1];
	draw_line_bresenham_3d_no_zfight(a.x, a.y, za, na, b.x, b.y, zb, nb, color);
	draw_line_bresenham_3d_no_zfight(b.x, b.y, zb, nb, c.x, c.y, zc, nc, color);
	draw_line_bresenham_3d_no_zfight(c.x, c.y, zc, nc, a.x, a.y, za, na, color);
}

void Graphics::draw_solid(const Triangle& triangle, const uint32& color)
{
	ZoneScoped; // for tracy

	glm::vec2 v0 = { triangle.vertices[0].x, triangle.vertices[0].y };
	glm::vec2 v1 = { triangle.vertices[1].x, triangle.vertices[1].y };
	glm::vec2 v2 = { triangle.vertices[2].x, triangle.vertices[2].y };
	float v0z = triangle.vertices[0].z;
	float v1z = triangle.vertices[1].z;
	float v2z = triangle.vertices[2].z;

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
	float area = Math3D::orient2d_f(v0, v1, v2);
	float inv_area = 1.0f / area;

	glm::vec2 p;
	// Loop over the bounding box and rasterize the triangle
	for (int x = min_x; x <= max_x; x++) {
		for (int y = min_y; y <= max_y; y++) {
			p.x = (float)x;
			p.y = (float)y;

			// Compute barycentric weights
			float alpha = Math3D::orient2d_f(v1, v2, p);
			float beta = Math3D::orient2d_f(v2, v0, p);
			float gamma = Math3D::orient2d_f(v0, v1, p);
			alpha *= inv_area;
			beta *= inv_area;
			gamma *= inv_area;

			// Check if the point is inside the triangle
			if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f)
			{
				// Interpolate depth values
				float depth = v0z * alpha + v1z * beta + v2z * gamma;

				// Check depth against z-buffer and only render if the pixel is
				// behind (front to back)
				int index = viewport.width * (viewport.height - y - 1) + x;
				float current_depth = depth_buffer[index];
				if (depth >= current_depth) {
					continue;
				}
				depth_buffer[index] = depth;

				// Set pixel color
				draw_pixel(x, y, color);
			}
		}
	}
}

void Graphics::draw_textured(const Triangle& triangle)
{
	ZoneScoped; // for tracy

	glm::vec2 v0 = { triangle.vertices[0].x, triangle.vertices[0].y };
	glm::vec2 v1 = { triangle.vertices[1].x, triangle.vertices[1].y };
	glm::vec2 v2 = { triangle.vertices[2].x, triangle.vertices[2].y };
	float v0z = triangle.vertices[0].z;
	float v1z = triangle.vertices[1].z;
	float v2z = triangle.vertices[2].z;
	tex2 uv0 = { triangle.texcoords[0].u,triangle.texcoords[0].v };
	tex2 uv1 = { triangle.texcoords[1].u,triangle.texcoords[1].v };
	tex2 uv2 = { triangle.texcoords[2].u,triangle.texcoords[2].v };
	float inv_w0 = triangle.inv_w[0];
	float inv_w1 = triangle.inv_w[1];
	float inv_w2 = triangle.inv_w[2];
	std::shared_ptr<Texture> texture = triangle.texture;

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
	float area = Math3D::orient2d_f(v0, v1, v2);
	float inv_area = 1.0f / area;

	glm::vec2 p;
	// Loop over the bounding box and rasterize the triangle
	for (int x = min_x; x <= max_x; x++) {
		for (int y = min_y; y <= max_y; y++) {
			p.x = (float)x + 0.5f;
			p.y = (float)y + 0.5f;

			// Compute barycentric weights
			float alpha = Math3D::orient2d_f(v1, v2, p);
			float beta = Math3D::orient2d_f(v2, v0, p);
			float gamma = Math3D::orient2d_f(v0, v1, p);
			alpha *= inv_area;
			beta *= inv_area;
			gamma *= inv_area;

			// Check if the point is inside the triangle
			if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f)
			{
				// Interpolate depth values
				float depth = v0z * alpha + v1z * beta + v2z * gamma;

				// Check depth against z-buffer and only render if the pixel is
				// behind (front to back)
				int index = viewport.width * (viewport.height - y - 1) + x;
				float current_depth = depth_buffer[index];
				if (depth >= current_depth) {
					continue;
				}
				depth_buffer[index] = depth;

				// Interpolate 1/w
				float A = inv_w0 * alpha;
				float B = inv_w1 * beta;
				float C = inv_w2 * gamma;
				float ABC = 1.0f / (A + B + C);

				// Interpolate texture coordinates with interpolated 1/w
				float u = uv0.u * A + uv1.u * B + uv2.u * C;
				float v = uv0.v * A + uv1.v * B + uv2.v * C;

				// Normalize
				u *= ABC;
				v *= ABC;

				// Look up texel value and set pixel color
				int tex_x = abs(int(u * (float)texture->width)) % texture->width;
				int tex_y = abs(int(v * (float)texture->height)) % texture->height;
				int tex_index = texture->width * (texture->height - tex_y - 1) + tex_x;
				draw_pixel(x, y, texture->pixels[tex_index]);
			}
		}
	}
}

void Graphics::draw_vertices(
	const Triangle& triangle, int point_size, const uint32& color)
{
	float offset = point_size * 0.5f;
	for (glm::vec4 vertex : triangle.vertices)
	{
		int x_pos = int(vertex.x - offset + 0.5f);
		int y_pos = int(vertex.y - offset + 0.5f);
		Graphics::draw_rect(x_pos, y_pos, point_size, point_size, color);
	}
}

void Graphics::draw_gizmo(const Gizmo& gizmo)
{
	// x axis
	draw_line_bresenham(
		int(gizmo.bases[0].points[0].x), int(gizmo.bases[0].points[0].y),
		int(gizmo.bases[0].points[1].x), int(gizmo.bases[0].points[1].y),
		Colors::YELLOW
	);
	// y axis
	draw_line_bresenham(
		int(gizmo.bases[1].points[0].x), int(gizmo.bases[1].points[0].y),
		int(gizmo.bases[1].points[1].x), int(gizmo.bases[1].points[1].y),
		Colors::MAGENTA
	);
	// z axis
	draw_line_bresenham(
		int(gizmo.bases[2].points[0].x), int(gizmo.bases[2].points[0].y),
		int(gizmo.bases[2].points[1].x), int(gizmo.bases[2].points[1].y),
		Colors::CYAN
	);
}

void Graphics::close_window()
{
	// Free the resources allocated
	delete[] framebuffer;
	delete[] depth_buffer;
	SDL_DestroyTexture(framebuffer_texture);
	// Close imgui
	gui->destroy();
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