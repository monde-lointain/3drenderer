#include "Graphics.h"

#include "GUI.h"
#include "../Logger/Logger.h"
#include "../Math/Math3D.h"
#include "../Mesh/Texture.h"
#include "../Triangle/Triangle.h"
#include "../Utils/Colors.h"
#include "../Utils/debug_helpers.h"
#include "../Renderer/Gizmo.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Viewport.h"
#include <tracy/tracy/Tracy.hpp>
#include <vectorclass/vectorclass.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

#ifdef _MSC_VER // Windows
#include <SDL.h>
#else // Linux
#include <SDL2/SDL.h>
#endif

void Graphics::initialize_framebuffer()
{
	framebuffer = new uint32[viewport->width * viewport->height];
	assert(framebuffer);

	framebuffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		viewport->width,
		viewport->height
	);
	assert(framebuffer_texture);

	depth_buffer = new float[viewport->width * viewport->height];
	assert(framebuffer);
}

void Graphics::init(SDL_Renderer* app_renderer, std::shared_ptr<Viewport> app_viewport)
{
	renderer = app_renderer;
	viewport = app_viewport;
}

void Graphics::clear_framebuffer(const uint32& color)
{
	ZoneScoped; // for tracy

	size_t i;
	size_t size = (size_t)viewport->width * viewport->height;
	size_t loop_count = size / 8;

	// Set up a register with 8 instances of the same color value
	Vec8ui v(color);

	// Eight pixels at a time until we have less than eight remaining
	for (i = 0; i < loop_count * 8; i += 8)
	{
		v.store(framebuffer + i);
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

	size_t i;
	size_t size = (size_t)viewport->width * viewport->height;
	size_t loop_count = size / 8;

	constexpr float MAX = std::numeric_limits<float>::max();

	// Set up a register with 8 float values set to the max possible value for a
	// float. We'll only render pixels if they are in front (less) of this value
	Vec8f v(MAX);

	// Eight pixels at a time until we have less than eight remaining
	for (i = 0; i < loop_count * 8; i += 8)
	{
		v.store(&depth_buffer[i]);
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
		viewport->width * sizeof(uint32));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer_texture, nullptr, nullptr);
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
	int index = viewport->width * (viewport->height - y - 1) + x;
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
		int index = viewport->width * (viewport->height - current_y - 1) + current_x;

		// Check if the pixel is within the bounds of the screen
		if (current_x >= 0 && current_x < viewport->width && 
			current_y >= 0 && current_y < viewport->height)
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
		int index = viewport->width * (viewport->height - current_y - 1) + current_x;

		// Check if the pixel is within the bounds of the screen
		if (current_x >= 0 && current_x < viewport->width &&
			current_y >= 0 && current_y < viewport->height)
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

	glm::ivec2 a = { lrintf(triangle.vertices[0].position.x), lrintf(triangle.vertices[0].position.y) };
	glm::ivec2 b = { lrintf(triangle.vertices[1].position.x), lrintf(triangle.vertices[1].position.y) };
	glm::ivec2 c = { lrintf(triangle.vertices[2].position.x), lrintf(triangle.vertices[2].position.y) };
	draw_line_bresenham(a.x, a.y, b.x, b.y, color);
	draw_line_bresenham(b.x, b.y, c.x, c.y, color);
	draw_line_bresenham(c.x, c.y, a.x, a.y, color);
}

void Graphics::draw_wireframe_3d(const Triangle& triangle, const uint32& color)
{
	ZoneScoped; // for tracy

	glm::ivec2 a = { lrintf(triangle.vertices[0].position.x), lrintf(triangle.vertices[0].position.y) };
	glm::ivec2 b = { lrintf(triangle.vertices[1].position.x), lrintf(triangle.vertices[1].position.y) };
	glm::ivec2 c = { lrintf(triangle.vertices[2].position.x), lrintf(triangle.vertices[2].position.y) };
	float za = triangle.vertices[0].position.z;
	float zb = triangle.vertices[1].position.z;
	float zc = triangle.vertices[2].position.z;
	glm::vec3 na = triangle.vertices[0].normal;
	glm::vec3 nb = triangle.vertices[1].normal;
	glm::vec3 nc = triangle.vertices[1].normal;
	draw_line_bresenham_3d_no_zfight(a.x, a.y, za, na, b.x, b.y, zb, nb, color);
	draw_line_bresenham_3d_no_zfight(b.x, b.y, zb, nb, c.x, c.y, zc, nc, color);
	draw_line_bresenham_3d_no_zfight(c.x, c.y, zc, nc, a.x, a.y, za, na, color);
}

typedef int32 fixed; // 28.4 fixed point format
constexpr int FIXED_BITS = 4;

void Graphics::draw_solid(
	const Triangle& triangle, uint32 color, EShadingMode shading_mode)
{
	ZoneScoped; // for tracy

	glm::vec2 v0 = { triangle.vertices[0].position.x, triangle.vertices[0].position.y };
	glm::vec2 v1 = { triangle.vertices[1].position.x, triangle.vertices[1].position.y };
	glm::vec2 v2 = { triangle.vertices[2].position.x, triangle.vertices[2].position.y };
	float v0z = triangle.vertices[0].position.z;
	float v1z = triangle.vertices[1].position.z;
	float v2z = triangle.vertices[2].position.z;
	float inv_w0 = triangle.vertices[0].position.w;
	float inv_w1 = triangle.vertices[1].position.w;
	float inv_w2 = triangle.vertices[2].position.w;
	float intensity = triangle.flat_value;
	float v0_intensity = triangle.vertices[0].gouraud;
	float v1_intensity = triangle.vertices[1].gouraud;
	float v2_intensity = triangle.vertices[2].gouraud;
	uint32 shaded = color;

	// Calculate triangle bounding box
	int min_x = std::min({ lrintf(v0.x), lrintf(v1.x), lrintf(v2.x) });
	int max_x = std::max({ lrintf(v0.x), lrintf(v1.x), lrintf(v2.x) });
	int min_y = std::min({ lrintf(v0.y), lrintf(v1.y), lrintf(v2.y) });
	int max_y = std::max({ lrintf(v0.y), lrintf(v1.y), lrintf(v2.y) });

	// Clip triangle bounding box to screen
	min_x = std::max(min_x, 0);
	max_x = std::min(max_x, viewport->width - 1);
	min_y = std::max(min_y, 0);
	max_y = std::min(max_y, viewport->height - 1);
	
	// Compute the inverse area of the triangle
	float area = Math3D::orient2d_f(v0, v1, v2);
	float inv_area2 = 1.0f / area;

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
			alpha *= inv_area2;
			beta *= inv_area2;
			gamma *= inv_area2;

			// Check if the point is inside the triangle
			if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f)
			{
				// Interpolate depth values
				float depth = v0z * alpha + v1z * beta + v2z * gamma;

				// Check depth against z-buffer and only render if the pixel is
				// behind (front to back)
				int index = viewport->width * (viewport->height - y - 1) + x;
				float current_depth = depth_buffer[index];
				if (depth >= current_depth) {
					continue;
				}
				depth_buffer[index] = depth;

				// Execute the pixel shader
				switch (shading_mode)
				{
					case NONE:
						// Set pixel color
						draw_pixel(x, y, shaded);
						break;
					case FLAT:
						shaded = apply_intensity(color, intensity);
						draw_pixel(x, y, shaded);
						break;
					case GOURAUD:
					{	 
						// Interpolate the Gouraud values across the triangle
						// (I can't notice any artifacts with perspective here,
						// so we can probably get away without interpolating)
						float pixel_intensity = v0_intensity * alpha
												+ v1_intensity * beta
												+ v2_intensity * gamma;

						// Get the new shaded value
						shaded = apply_intensity(color, pixel_intensity);

						// Render the pixel
						draw_pixel(x, y, shaded);
					}
					break;
				}
			}
		}
	}
}

void Graphics::draw_textured(const Triangle& triangle, EShadingMode shading_mode)
{
	ZoneScoped; // for tracy

	// x, y coordinates in screen space (x/w, y/w)
	glm::vec2 v0 = { triangle.vertices[0].position.x, triangle.vertices[0].position.y };
	glm::vec2 v1 = { triangle.vertices[1].position.x, triangle.vertices[1].position.y };
	glm::vec2 v2 = { triangle.vertices[2].position.x, triangle.vertices[2].position.y };

	// z coordinates in screen space (z/w)
	float v0z = triangle.vertices[0].position.z;
	float v1z = triangle.vertices[1].position.z;
	float v2z = triangle.vertices[2].position.z;

	// texture coordinates (non-interpolated)
	tex2 uv0 = { triangle.vertices[0].uv.u,triangle.vertices[0].uv.v };
	tex2 uv1 = { triangle.vertices[1].uv.u,triangle.vertices[1].uv.v };
	tex2 uv2 = { triangle.vertices[2].uv.u,triangle.vertices[2].uv.v };

	// 1/w
	float inv_w0 = triangle.vertices[0].position.w;
	float inv_w1 = triangle.vertices[1].position.w;
	float inv_w2 = triangle.vertices[2].position.w;

	// face intensity for flat shading
	float intensity = triangle.flat_value;

	// vertex intensities for vertex shading
	float v0_intensity = triangle.vertices[0].gouraud;
	float v1_intensity = triangle.vertices[1].gouraud;
	float v2_intensity = triangle.vertices[2].gouraud;

	std::shared_ptr<Texture> texture = triangle.texture;

	// Calculate triangle bounding box
	int min_x = std::min({ lrintf(v0.x), lrintf(v1.x), lrintf(v2.x) });
	int max_x = std::max({ lrintf(v0.x), lrintf(v1.x), lrintf(v2.x) });
	int min_y = std::min({ lrintf(v0.y), lrintf(v1.y), lrintf(v2.y) });
	int max_y = std::max({ lrintf(v0.y), lrintf(v1.y), lrintf(v2.y) });

	// Clip triangle bounding box to screen
	min_x = std::max(min_x, 0);
	max_x = std::min(max_x, viewport->width - 1);
	min_y = std::max(min_y, 0);
	max_y = std::min(max_y, viewport->height - 1);

	//// Compute the edge equations for each of the three line segments on the triangle (Ax + By + C)
	//float A01 = v0.y - v1.y;
	//float A12 = v1.y - v2.y;
	//float A20 = v2.y - v0.y;
	//float B01 = v1.x - v0.x;
	//float B12 = v2.x - v1.x;
	//float B20 = v0.x - v2.x;
	//float C01 = v0.x * v1.y - v1.x * v0.y;
	//float C12 = v1.x * v2.y - v2.x * v1.y;
	//float C20 = v2.x * v0.y - v0.x * v2.y;

	//float C1 = A01 * v0.x - B01 * v0.y;
	//float C2 = A12 * v1.x - B12 * v1.y;
	//float C3 = A20 * v2.x - B20 * v2.y;

	//float Cy1 = C1 + B01 * (float)max_y - A01 * (float)min_x;
	//float Cy2 = C2 + B12 * (float)max_y - A12 * (float)min_x;
	//float Cy3 = C3 + B20 * (float)max_y - A20 * (float)min_x;

	//glm::vec2 p0 = glm::vec2(float(min_x), float(min_y));
	//float w0_row = Math3D::orient2d_f(v1, v2, p0);
	//float w1_row = Math3D::orient2d_f(v2, v0, p0);
	//float w2_row = Math3D::orient2d_f(v0, v1, p0);


	// Compute the inverse area of the triangle
	float area2 = Math3D::orient2d_f(v0, v1, v2);
	float inv_area2 = 1.0f / area2;

	// Normalize the screen space z coordinates (TODO: do we need to do the same with 1/w?)
	v0z *= inv_area2;
	v1z *= inv_area2;
	v2z *= inv_area2;

	inv_w0 *= inv_area2;
	inv_w1 *= inv_area2;
	inv_w2 *= inv_area2;


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

			// Check if the point is inside the triangle
			if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f)
			{
				// Interpolate depth values
				float depth = v0z * alpha + v1z * beta + v2z * gamma;

				// Check depth against z-buffer and only render if the pixel is
				// behind (front to back)
				int index = viewport->width * (viewport->height - y - 1) + x;
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
				int tex_x = abs(lrintf(u * (float)texture->width)) % texture->width;
				int tex_y = abs(lrintf(v * (float)texture->height)) % texture->height;
				int tex_index = texture->width * (texture->height - tex_y - 1) + tex_x;
				uint32 color = texture->pixels[tex_index];

				// Execute the pixel shader
				switch (shading_mode)
				{
					case NONE:
						// Set pixel color
						draw_pixel(x, y, color);
						break;
					case FLAT:
						color = apply_intensity(color, intensity);
						draw_pixel(x, y, color);
						break;
					case GOURAUD:
					{
						// Interpolate intensity with interpolated 1/w
						// NOTE: Although we don't interpolate in draw_solid, we
						// do here because we already have to compute the
						// interpolation constants to do the texture mapping
						// anyways
						float pixel_intensity = v0_intensity * A
												+ v1_intensity * B
												+ v2_intensity * C;

						// Normalize
						pixel_intensity *= ABC;

						// Get the new shaded value
						color = apply_intensity(color, pixel_intensity);

						// Render the pixel
						draw_pixel(x, y, color);
					}
					break;
				}
			}
		}
	}
}

/**
 * Original rasterization algorithm kept for reference purposes. This one had
 * subpixel precision but was much slower
 */
//void Graphics::draw_textured(const Triangle& triangle, EShadingMode shading_mode)
//{
//	ZoneScoped; // for tracy
//
//	glm::vec2 v0 = { triangle.vertices[0].position.x, triangle.vertices[0].position.y };
//	glm::vec2 v1 = { triangle.vertices[1].position.x, triangle.vertices[1].position.y };
//	glm::vec2 v2 = { triangle.vertices[2].position.x, triangle.vertices[2].position.y };
//	float v0z = triangle.vertices[0].position.z;
//	float v1z = triangle.vertices[1].position.z;
//	float v2z = triangle.vertices[2].position.z;
//	tex2 uv0 = { triangle.vertices[0].uv.u,triangle.vertices[0].uv.v };
//	tex2 uv1 = { triangle.vertices[1].uv.u,triangle.vertices[1].uv.v };
//	tex2 uv2 = { triangle.vertices[2].uv.u,triangle.vertices[2].uv.v };
//	float inv_w0 = triangle.vertices[0].position.w;
//	float inv_w1 = triangle.vertices[1].position.w;
//	float inv_w2 = triangle.vertices[2].position.w;
//	std::shared_ptr<Texture> texture = triangle.texture;
//	float intensity = triangle.flat_value;
//	float v0_intensity = triangle.vertices[0].gouraud;
//	float v1_intensity = triangle.vertices[1].gouraud;
//	float v2_intensity = triangle.vertices[2].gouraud;
//
//	// Calculate triangle bounding box
//	int min_x = std::min({ lrintf(v0.x), lrintf(v1.x), lrintf(v2.x) });
//	int max_x = std::max({ lrintf(v0.x), lrintf(v1.x), lrintf(v2.x) });
//	int min_y = std::min({ lrintf(v0.y), lrintf(v1.y), lrintf(v2.y) });
//	int max_y = std::max({ lrintf(v0.y), lrintf(v1.y), lrintf(v2.y) });
//
//	// Clip triangle bounding box to screen
//	min_x = std::max(min_x, 0);
//	max_x = std::min(max_x, viewport->width - 1);
//	min_y = std::max(min_y, 0);
//	max_y = std::min(max_y, viewport->height - 1);
//
//	// Compute the inverse area of the triangle
//	float area = Math3D::orient2d_f(v0, v1, v2);
//	float inv_area2 = 1.0f / area;
//
//	glm::vec2 p;
//	// Loop over the bounding box and rasterize the triangle
//	for (int x = min_x; x <= max_x; x++) {
//		for (int y = min_y; y <= max_y; y++) {
//			p.x = (float)x + 0.5f;
//			p.y = (float)y + 0.5f;
//
//			// Compute barycentric weights
//			float alpha = Math3D::orient2d_f(v1, v2, p);
//			float beta = Math3D::orient2d_f(v2, v0, p);
//			float gamma = Math3D::orient2d_f(v0, v1, p);
//			alpha *= inv_area2;
//			beta *= inv_area2;
//			gamma *= inv_area2;
//
//			// Check if the point is inside the triangle
//			if (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f)
//			{
//				// Interpolate depth values
//				float depth = v0z * alpha + v1z * beta + v2z * gamma;
//
//				// Check depth against z-buffer and only render if the pixel is
//				// behind (front to back)
//				int index = viewport->width * (viewport->height - y - 1) + x;
//				float current_depth = depth_buffer[index];
//				if (depth >= current_depth) {
//					continue;
//				}
//				depth_buffer[index] = depth;
//
//				// Interpolate 1/w
//				float A = inv_w0 * alpha;
//				float B = inv_w1 * beta;
//				float C = inv_w2 * gamma;
//				float ABC = 1.0f / (A + B + C);
//
//				// Interpolate texture coordinates with interpolated 1/w
//				float u = uv0.u * A + uv1.u * B + uv2.u * C;
//				float v = uv0.v * A + uv1.v * B + uv2.v * C;
//
//				// Normalize
//				u *= ABC;
//				v *= ABC;
//
//				// Look up texel value and set pixel color
//				int tex_x = abs(lrintf(u * (float)texture->width)) % texture->width;
//				int tex_y = abs(lrintf(v * (float)texture->height)) % texture->height;
//				int tex_index = texture->width * (texture->height - tex_y - 1) + tex_x;
//				uint32 color = texture->pixels[tex_index];
//
//				// Execute the pixel shader
//				switch (shading_mode)
//				{
//				case NONE:
//					// Set pixel color
//					draw_pixel(x, y, color);
//					break;
//				case FLAT:
//					color = apply_intensity(color, intensity);
//					draw_pixel(x, y, color);
//					break;
//				case GOURAUD:
//				{
//					// Interpolate intensity with interpolated 1/w
//					// NOTE: Although we don't interpolate in draw_solid, we
//					// do here because we already have to compute the
//					// interpolation constants to do the texture mapping
//					// anyways
//					float pixel_intensity = v0_intensity * A
//						+ v1_intensity * B
//						+ v2_intensity * C;
//
//					// Normalize
//					pixel_intensity *= ABC;
//
//					// Get the new shaded value
//					color = apply_intensity(color, pixel_intensity);
//
//					// Render the pixel
//					draw_pixel(x, y, color);
//				}
//				break;
//				}
//			}
//		}
//	}
//}

void Graphics::draw_vertices(
	const Triangle& triangle, int point_size, const uint32& color)
{
	float offset = point_size * 0.5f;
	for (const Vertex& vertex : triangle.vertices)
	{
		int x_pos = lrintf(vertex.position.x - offset + 0.5f);
		int y_pos = lrintf(vertex.position.y - offset + 0.5f);
		Graphics::draw_rect(x_pos, y_pos, point_size, point_size, color);
	}
}

void Graphics::draw_gizmo(const Gizmo& gizmo)
{
	// x axis
	draw_line_bresenham(
		lrintf(gizmo.bases[0].points[0].x), lrintf(gizmo.bases[0].points[0].y),
		lrintf(gizmo.bases[0].points[1].x), lrintf(gizmo.bases[0].points[1].y),
		Colors::YELLOW
	);
	// y axis
	draw_line_bresenham(
		lrintf(gizmo.bases[1].points[0].x), lrintf(gizmo.bases[1].points[0].y),
		lrintf(gizmo.bases[1].points[1].x), lrintf(gizmo.bases[1].points[1].y),
		Colors::MAGENTA
	);
	// z axis
	draw_line_bresenham(
		lrintf(gizmo.bases[2].points[0].x), lrintf(gizmo.bases[2].points[0].y),
		lrintf(gizmo.bases[2].points[1].x), lrintf(gizmo.bases[2].points[1].y),
		Colors::CYAN
	);
}

void Graphics::free_framebuffer()
{
	// Free the resources allocated
	delete[] framebuffer;
	delete[] depth_buffer;
	SDL_DestroyTexture(framebuffer_texture);
}

bool Graphics::is_in_viewport(int x, int y)
{
	bool x_in_viewport = x >= 0 && x < viewport->width;
	bool y_in_viewport = y >= 0 && y < viewport->height;
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

uint32 Graphics::apply_intensity(const uint32& color, const float& intensity)
{
	// Unpack and convert to float
	float r = (float)((color >> 16) & 0xFF);
	float g = (float)((color >> 8) & 0xFF);
	float b = (float)((color >> 0) & 0xFF);
	float a = (float)((color >> 24) & 0xFF);

	// Multiply the color channels by the intensity
	r *= intensity;
	g *= intensity;
	b *= intensity;
	a *= 1.0f; // give the hint to the compiler to vectorize

	// Repack
	uint32 out = (((uint32)(r + 0.5f) << 16) |
		          ((uint32)(g + 0.5f) << 8) |
		          ((uint32)(b + 0.5f) << 0) |
		          ((uint32)(a + 0.5f) << 24));
	return out;
}

bool Graphics::is_top_left(const glm::ivec2& a, const glm::ivec2& b)
{
	bool is_top = (a.y == b.y) && (a.x < b.x);
	bool is_left = a.y > b.y;
	return is_top || is_left;
}
