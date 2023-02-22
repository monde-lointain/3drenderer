#pragma once

#include "../Light/Light.h"
#include "../Mesh/Mesh.h"
#include "../Triangle/Triangle.h"
#include "../Renderer/Camera.h"
#include "../Renderer/Gizmo.h"
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

struct Line3D;
struct Viewport;
struct Triangle;

struct World
{
	void load_level(const std::shared_ptr<Viewport>& viewport);
	void update();

	Camera camera;
	std::vector<std::unique_ptr<Mesh>> meshes; // triangles
	/**
	 * The x axis of the gizmo is drawn in yellow. The y axis is drawn in
	 * magenta, and the z axis cyan.
	 */
	Gizmo gizmo; // three lines
	Light light; // one line

	std::vector<Triangle> triangles_in_scene;
	std::vector<Line3D> lines_in_scene;

	glm::mat4 modelview_matrix;

	float x = 0.1f;

	void transform_mesh(Mesh* mesh);
	void transform_gizmo();
	void transform_light_direction_vector();
	void compute_light_intensity(Triangle& transformed_triangle) const;
};

