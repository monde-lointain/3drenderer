#pragma once

#include "../Renderer/ShadingMode.h"
#include "../Utils/3d_types.h"
#include "../Utils/Colors.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>

struct Gizmo;
struct SDL_Renderer;
struct SDL_Texture;
struct Triangle;
struct Viewport;

// TODO: This should not be a structure. We don't access these functions
// anywhere else but from the renderer. It's safe just to keep them as is in
// their own header file without a separate graphics class
struct Graphics
{
	void initialize_framebuffer();
	void free_framebuffer();
	void init(SDL_Renderer* app_renderer, std::shared_ptr<Viewport> app_viewport);
	void clear_framebuffer(const uint32& color);
	void clear_z_buffer();
	void update_framebuffer();
	void render_frame();
	void draw_pixel(const int& x, const int& y, const uint32& color);
	void draw_rect(const int& x, const int& y, const int& width,
		const int& height, const uint32& color);
	void draw_line_dda(const int& x0, const int& y0, const int& x1,
		const int& y1, const uint32& color);
	void draw_line_bresenham(const int& x0, const int& y0, const int& x1,
		const int& y1, const uint32& color);
	void draw_line_bresenham_3d(const int& x1, const int& y1, const float& z1,
		const int& x2, const int& y2, const float& z2, const uint32& color);
	void draw_line_bresenham_3d_no_zfight(const int& x1, const int& y1,
		const float& z1, const glm::vec3& n1, const int& x2, const int& y2,
		const float& z2, const glm::vec3& n2, const uint32& color);
	void draw_wireframe(const Triangle& triangle, const uint32& color);
	void draw_wireframe_3d(const Triangle& triangle, const uint32& color);
	void draw_solid(const Triangle& triangle, uint32 color, EShadingMode shading_mode);
	void draw_textured(const Triangle& triangle, EShadingMode shading_mode);
	void draw_vertices(const Triangle& triangle, int point_size, const uint32& color);
	void draw_gizmo(const Gizmo& gizmo);
	bool is_in_viewport(int x, int y);
	bool is_top_left(const glm::ivec2& a, const glm::ivec2& b);
	uint32 get_zbuffer_color(float val);
	uint32 apply_intensity(const uint32& color, const float& intensity);

	std::shared_ptr<Viewport> viewport;
	uint32* framebuffer = nullptr;
	SDL_Texture* framebuffer_texture = nullptr;
	float* depth_buffer = nullptr;
	SDL_Renderer* renderer;
};
