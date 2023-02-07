#pragma once

#include "../Misc/3d_types.h"
#include "../Renderer/Viewport.h"
#include <intrin.h>

struct Gizmo;
struct Triangle;

namespace Graphics
{
	bool initialize_window();
	void initialize_framebuffer();
	void clear_framebuffer(uint32 color);
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
	void draw_line_bresenham_3d(const int& x1, const int& y1, const float& w1,
		const int& x2, const int& y2, const float& w2, const uint32& color);
	void draw_wireframe(const Triangle& triangle, const uint32& color);
	void draw_wireframe_3d(const Triangle& triangle, const uint32& color);
	void draw_solid(const Triangle& triangle, const uint32& color);
	void draw_textured(const Triangle& triangle);
	void draw_vertices(const Triangle& triangle, int point_size, const uint32& color);
	void draw_gizmo(const Gizmo& gizmo);
	void close_window();
	bool is_in_viewport(int x, int y);
	uint32 get_zbuffer_color(float val);

	extern Viewport viewport;
};
