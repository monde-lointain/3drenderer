#pragma once

#include "RenderMode.h"
#include "ShadingMode.h"
#include "../Clipping/Clipper.h"
#include "../Graphics/Graphics.h"
#include "../Utils/3d_types.h"
#include <memory>
#include <vector>

struct SDL_Texture;
struct Viewport;
struct Window;
struct World;

struct Renderer
{
	void initialize(std::shared_ptr<Window> app_window,
		            std::shared_ptr<Viewport> app_viewport,
		            std::shared_ptr<World> app_world);
	void render();
	void display_frame();
	void destroy();

	void set_render_mode(ERenderMode mode);
	void set_shading_mode(EShadingMode mode);

	std::shared_ptr<Viewport> viewport;
	std::shared_ptr<Window> window;
	std::shared_ptr<World> world;

	std::unique_ptr<Triangle[]> triangles_to_rasterize;

	ERenderMode render_mode;
	EShadingMode shading_mode;
	bool display_face_normals;
	bool backface_culling;

	Clipper clipper;
	std::unique_ptr<Graphics> graphics;

	void render_triangles_in_scene();
	void render_lines();
	void draw_face_normal(const Triangle& triangle);
};