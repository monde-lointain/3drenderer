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

// TODO: This should not be a structure. We don't access these functions
// anywhere else but from the renderer. It's safe just to keep them as is in
// their own header file without a separate graphics class
struct Graphics
{
	/** Initialization and freeing of resources */
	void init(SDL_Renderer* app_renderer, std::shared_ptr<Viewport> app_viewport);
	void initialize_framebuffer();
	void free_framebuffer() const;

	/** Clearing and updating the buffers */
	void clear_framebuffer(uint32 color) const;
	void clear_z_buffer() const;
	void update_framebuffer() const;
	void render_frame() const;

	/** Basic drawing algorithms */
	void draw_pixel(const glm::ivec2& p, uint32 color) const;
	void draw_rect(const SDL_Rect& rect, uint32 color) const;

	/** Line drawing algorithms */
	void draw_line_dda(
		const glm::ivec2& start, 
		const glm::ivec2& end, 
		uint32 color
	) const;
	void draw_line_bresenham(
		const glm::ivec2& start, 
		const glm::ivec2& end, 
		uint32 color
	) const;
	void draw_line_bresenham_3d(
		const glm::ivec2& start, 
		const glm::ivec2& end,
		float start_z, 
		float end_z, 
		uint32 color
	) const;
	void draw_line_bresenham_3d_no_zfight(
		const glm::ivec2& start,
		const glm::ivec2& end, 
		float start_z, 
		float end_z,
		const glm::vec3& start_normal, 
		const glm::vec3& end_normal,
		uint32 color
	) const;

	/** Wireframe drawing algorithms */
	void draw_wireframe(const Triangle& triangle, uint32 color) const;
	void draw_wireframe_3d(const Triangle& triangle, uint32 color) const;

	/** Solid drawing algorithms */
	void draw_solid(
		const Triangle& triangle, 
		uint32 color, 
		EShadingMode shading_mode
	) const;
	void draw_textured(
		const Triangle& triangle, 
		EShadingMode shading_mode
	) const;

	/** Misc. drawing algorithms */
	void draw_vertices(
		const Triangle& triangle, 
		int point_size, 
		uint32 color
	) const;
	void draw_gizmo(const Gizmo& gizmo) const;

	/** Misc. functions */
	[[nodiscard]] bool is_in_viewport(const glm::ivec2& p) const;
	static bool is_top_left(const glm::ivec2& a, const glm::ivec2& b);
	static uint32 get_zbuffer_color(float val);
	static uint32 apply_intensity(uint32 color, float intensity);

	std::shared_ptr<Viewport> viewport;
	SDL_Renderer* renderer;
	// These can be declared in the cpp
	uint32* framebuffer = nullptr;
	SDL_Texture* framebuffer_texture = nullptr;
	float* depth_buffer = nullptr;
};