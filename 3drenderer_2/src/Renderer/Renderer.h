#pragma once

#include <array>
#include <memory>

#include "RenderMode.h"
#include "ShadingMode.h"
#include "../Utils/Constants.h"

struct SDL_Texture;
struct Triangle;
struct Viewport;
struct Window;
struct World;

struct Renderer
{
	void initialize(std::shared_ptr<Window> app_window,
		            std::shared_ptr<Viewport> app_viewport,
		            std::shared_ptr<World> app_world);
	void render();
	static void display_frame();
	static void destroy();

	void set_render_mode(ERenderMode mode);
	void set_shading_mode(EShadingMode mode);

	std::shared_ptr<Viewport> viewport;
	std::shared_ptr<Window> window;
	std::shared_ptr<World> world;

	std::unique_ptr<std::array<Triangle, MAX_TRIANGLES>> triangles_to_rasterize;

	ERenderMode render_mode;
	EShadingMode shading_mode;
	bool display_face_normals;
	bool backface_culling;

	void render_triangles_in_scene();
	void render_lines() const;
	void rasterize_triangle(Triangle& triangle) const;
	void draw_face_normal(const Triangle& triangle) const;
};