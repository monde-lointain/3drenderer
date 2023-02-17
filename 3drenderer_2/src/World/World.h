#pragma once

#include "../Light/Light.h"
#include "../Triangle/Triangle.h"
#include "../Renderer/Camera.h"
#include "../Renderer/Gizmo.h"
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

struct Line3D;
struct Mesh;
struct Viewport;
struct Triangle;

struct World
{
	void load_level(std::shared_ptr<Viewport> viewport);
	void update();
	void destroy();

	Camera camera;
	std::vector<Mesh*> meshes; // triangles
	/**
	 * The x axis of the gizmo is drawn in yellow. The y axis is drawn in
	 * magenta, and the z axis cyan.
	 */
	Gizmo gizmo; // three lines
	Light light; // one line

	std::vector<Triangle> triangles_in_scene;
	std::vector<Line3D> lines_in_scene;

	// TODO: Add this to the mesh class when we add the projection matrix
	// multiply here. We'll just have transformation and projection all happen
	// in the same loop for each mesh so we can easily multiply each meshes
	// modelview matrix by the projection matrix without having to keep track of
	// which triangles belonged to what mesh
	glm::mat4 modelview_matrix;

	float x = 0.1f;

	void transform_meshes();
	void transform_mesh(Mesh* mesh);
	void transform_lines();
	void compute_face_normal(Triangle& triangle);
	void compute_light_intensity(Triangle& triangle);
};

