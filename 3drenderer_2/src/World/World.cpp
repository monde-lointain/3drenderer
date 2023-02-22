#include "World.h"

#include <tracy/tracy/Tracy.hpp>

#include "../Logger/Logger.h"
#include "../Math/Math3D.h"
#include "../Viewport/Viewport.h"
#include "../Utils/string_ops.h"

void World::load_level(const std::unique_ptr<Viewport>& viewport)
{
	// TODO: Set the starting camera/light params. Load the starting mesh
	// Set the camera position
	camera.translation = glm::vec3(0.0f, 8.0f, 15.0f);
	camera.rotation.yaw = -90.0f;
	camera.direction = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.update();

	// Create the camera view matrix
	camera.set_view();

	// Set the camera projection attributes
	camera.fov = 60.0f;
	camera.aspect = (float)viewport->width / (float)viewport->height;
	camera.znear = 0.1f;
	camera.zfar = 100.0f;

	// Create the camera projection matrix
	camera.set_projection(PERSPECTIVE);

	// Create the light
	light.translation = glm::vec3(0.0f, 20.0f, 0.0f);
	light.rotation = rot3(0.0f, 0.0f, 0.0f);
	light.intensity = 1.0f;

	// Load the starting mesh
	std::unique_ptr<Mesh> mesh = create_mesh("assets/models/cube/cube.obj");

	// Add the mesh to the array of meshes
	meshes.push_back(std::move(mesh));
}

void World::update()
{
	ZoneScoped; // for tracy

	// Update the position and rotation of the camera
	camera.update();
	// Update the view matrix
	camera.set_view();
	// Update the position and rotation of the light
	light.update();

	const glm::vec3 scale(10.0f);
	const rot3 rotation(0.0f, x, 0.0f);
	//const glm::vec3 translation(0.0f, 0.0f, 0.0f);

	for (const std::unique_ptr<Mesh>& mesh : meshes)
	{
		mesh->scale = scale;
		mesh->rotate(rotation);
		mesh->update();
		modelview_matrix = camera.view_matrix * mesh->transform;
		transform_gizmo();
		transform_mesh(mesh.get()); // Passing the raw pointer
	}

	transform_light_direction_vector();
}

// NOTE: The raw pointers will not be managed after being created, so make sure
// not to access them after the unique_ptr goes out of scope!
void World::transform_mesh(Mesh* mesh)
{
	// Line segments for computing the face normal
	glm::vec3 ab, ca;

	// Loop over all the triangles in the mesh
	for (const Triangle& triangle : mesh->triangles)
	{
		/* Local space */
		Triangle transformed_triangle = triangle;

		// Compute the face normal of the triangle
		ab = glm::vec3(transformed_triangle.vertices[1].position - transformed_triangle.vertices[0].position);
		ca = glm::vec3(transformed_triangle.vertices[2].position - transformed_triangle.vertices[0].position);
		glm::vec3 face_normal = glm::cross(ab, ca);
		face_normal = glm::normalize(face_normal);

		// Rotate the face normal
		Math3D::rotate_normal(face_normal, mesh->transform);
		transformed_triangle.face_normal = face_normal;

		for (Vertex& vertex : transformed_triangle.vertices)
		{
			// Transform the vertices by the model matrix
			Math3D::transform_point(vertex.position, mesh->transform);
			// Rotate the vertex normals by the model matrix
			Math3D::rotate_normal(vertex.normal, mesh->transform);
		}

		/* World space */
		compute_light_intensity(transformed_triangle);

		for (Vertex& vertex : transformed_triangle.vertices)
		{
			// Transform the vertices by the concatenated view-projection matrix
			Math3D::transform_point(vertex.position, camera.vp_matrix);
			// Rotate the vertex normals by the concatenated view-projection matrix
			Math3D::rotate_normal(vertex.normal, camera.vp_matrix);
		}

		/* Clip space */
		// Add the transformed triangle to the bin of triangles to be rendered
		triangles_in_scene.push_back(transformed_triangle);
	}
}

void World::transform_gizmo()
{
	for (Line3D& basis : gizmo.bases)
	{
		for (glm::vec4& point : basis.points)
		{
			Math3D::transform_point(point, modelview_matrix);
		}
		lines_in_scene.push_back(basis);
	}
}

void World::transform_light_direction_vector()
{
	// Create the model-view matrix for the light direction vector
	modelview_matrix = camera.view_matrix * light.transform;

	// Transform the points of the light direction vector by the model-view matrix
	for (glm::vec4& point : light.direction_vector.points)
	{
		Math3D::transform_point(point, modelview_matrix);
	}
	lines_in_scene.push_back(light.direction_vector);

	Logger::info(LOG_CATEGORY_LIGHT, "Direction vector start (camera space): " + vec4_to_string(light.direction_vector.points[0]));
	Logger::info(LOG_CATEGORY_LIGHT, "Direction vector end (camera space): " + vec4_to_string(light.direction_vector.points[1]));
	Logger::info(LOG_CATEGORY_LIGHT, "Light transform matrix:\n" + mat4_to_string(light.transform));
	Logger::info(LOG_CATEGORY_LIGHT, "Camera view matrix:\n" + mat4_to_string(camera.view_matrix));
	Logger::info(LOG_CATEGORY_LIGHT, "Light model-view matrix:\n" + mat4_to_string(modelview_matrix));
}

void World::compute_light_intensity(Triangle& triangle) const
{
	// Using the face normal, assign a light intensity value to each face using the dot product
	const glm::vec3 light_direction = light.direction;
	float intensity = -glm::dot(triangle.face_normal, light_direction);

	// Map intensity value from [-1, 1] to [0, 1]
	intensity = (intensity + 1.0f) * 0.5f;
	triangle.flat_value = intensity;

	// Compute the intensity for each vertex
	float v0_intensity = -glm::dot(triangle.vertices[0].normal, light_direction);
	float v1_intensity = -glm::dot(triangle.vertices[1].normal, light_direction);
	float v2_intensity = -glm::dot(triangle.vertices[2].normal, light_direction);

	// Map intensity values from [-1, 1] to [0, 1]
	v0_intensity = (v0_intensity + 1.0f) * 0.5f;
	v1_intensity = (v1_intensity + 1.0f) * 0.5f;
	v2_intensity = (v2_intensity + 1.0f) * 0.5f;

	triangle.vertices[0].gouraud = v0_intensity;
	triangle.vertices[1].gouraud = v1_intensity;
	triangle.vertices[2].gouraud = v2_intensity;
}
