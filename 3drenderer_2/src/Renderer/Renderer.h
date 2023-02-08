#pragma once

#include "Camera.h"
#include "Gizmo.h"
#include "../Utils/3d_types.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

struct Gizmo;
struct Mesh;
struct Triangle;

enum ERenderMode
{
	VERTICES_ONLY,
	WIREFRAME,
	WIREFRAME_VERTICES,
	SOLID,
	SOLID_WIREFRAME,
	TEXTURED,
	TEXTURED_WIREFRAME,
};

struct Renderer
{
	Renderer();
	~Renderer();

	void setup();
	void run();
	void process_input();
	void update();
	void render();
	void destroy();

	void set_render_mode(ERenderMode mode);

	bool is_running;
	float current_fps;
	float seconds_per_frame;

	ERenderMode render_mode;
	bool display_face_normals;
	bool backface_culling;

	float x = 0.0f;

	Camera camera;
	glm::mat4 projection_matrix;
	/** The x axis of the gizmo is drawn in yellow. The y axis is magenta, and the z axis cyan. */
	Gizmo gizmo;
	std::vector<Mesh*> meshes;
	std::vector<Triangle> triangles_in_scene;

	void transform_triangles(Mesh* mesh, const glm::mat4& modelview_matrix);
	void transform_gizmo(const glm::mat4& modelview_matrix);
	void project_triangle(Triangle& triangle);
	void project_gizmo();
	void render_triangles_in_scene();
	void draw_face_normal(const Triangle& triangle);
	void compute_face_normal(Triangle& transformed_triangle);
};