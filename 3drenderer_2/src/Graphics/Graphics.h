#pragma once

#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../Renderer/ShadingMode.h"
#include "../Utils/3d_types.h"

struct Gizmo;
struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;
struct Triangle;
struct Viewport;

/** Initialization and freeing of resources */
void graphics_init(SDL_Renderer* renderer_, Viewport* viewport_);
void initialize_framebuffer();
void free_framebuffer();

/** Clearing and updating the buffers */
void clear_framebuffer(uint32 color);
void clear_z_buffer();
void update_framebuffer();
void render_frame();

/** Basic drawing algorithms */
void draw_pixel(const glm::ivec2& p, uint32 color);
void draw_rect(const SDL_Rect& rect, uint32 color);

/** Line drawing algorithms */
void draw_line_dda(
	const glm::ivec2& start, 
	const glm::ivec2& end, 
	uint32 color
);
void draw_line_bresenham(
	const glm::ivec2& start, 
	const glm::ivec2& end, 
	uint32 color
);
void draw_line_bresenham_3d(
	const glm::ivec2& start, 
	const glm::ivec2& end,
	float start_z, 
	float end_z, 
	uint32 color
);
void draw_line_bresenham_3d_no_zfight(
	const glm::ivec2& start,
	const glm::ivec2& end, 
	float start_z, 
	float end_z,
	const glm::vec3& start_normal, 
	const glm::vec3& end_normal,
	uint32 color
);

/** Wireframe drawing algorithms */
void draw_wireframe(const Triangle& triangle, uint32 color);
void draw_wireframe_3d(const Triangle& triangle, uint32 color);

/** Solid drawing algorithms */
void draw_solid(const Triangle& triangle, uint32 color, EShadingMode shading_mode);
void draw_textured(const Triangle& triangle, EShadingMode shading_mode);

/** Misc. drawing algorithms */
void draw_vertices(const Triangle& triangle, int point_size, uint32 color);
void draw_gizmo(const Gizmo& gizmo);

/** Misc. functions */
bool is_in_viewport(const glm::ivec2& p);
bool is_top_left(const glm::ivec2& a, const glm::ivec2& b);
uint32 get_zbuffer_color(float val);
uint32 apply_intensity(uint32 color, float intensity);