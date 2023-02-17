#include "World.h"

#include "../Logger/Logger.h"
#include "../Math/Math3D.h"
#include "../Mesh/Mesh.h"
#include "../Renderer/Viewport.h"
#include "../Utils/string_ops.h"
#include <tracy/tracy/Tracy.hpp>

void World::load_level(std::shared_ptr<Viewport> viewport)
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
	Mesh* mesh = create_mesh("assets/models/robot/robot.obj");

	// Add the mesh to the array of meshes
	meshes.push_back(mesh);
}

void World::update()
{
	ZoneScoped; // for tracy

	/**
	 * TODO: 
	 * When we're sure everything with the pipeline is finalized we can move
	 * the projection matrix into here too and multiply all the entities
	 * modelview matrices by that as well, so we don't have to loop over all of
	 * them twice. Everything after from there on though (starting with
	 * clipping) will be the responsibility of the renderer
	 */

	// Update the position and rotation of the camera
	camera.update();
	// Update the view matrix
	camera.set_view();

	//light.rotation.pitch = 0.0f;
	// Update the position and rotation of the light
	light.update();

	//for (Mesh*& mesh : meshes)
	//{
	//	// Concatenate the world and view matrices into the modelview matrix
	//	modelview_matrix = camera.view_matrix * mesh->transform;
	//}

	// Update the positions of all lines in the scene
	transform_lines();
	transform_meshes(); // Lighting computations happen here, per triangle

	// TODO: Where should lighting computations happen?
	// Goals: We want to be able to support flat shading and Gouraud. We only
	// need to know:
	// A: The vertex and face normals, and
	// B: The direction of the light, so we can calculate the intensity factor
	// We should just do it in camera space, after applying the modelview
	// matrices for everything
}

void World::destroy()
{
	// Don't need to destroy the camera, gizmo and light because of RAII. Or any 
	for (Mesh*& mesh : meshes)
	{
		delete mesh;
	}
}


// NOTE: This is what transform_meshes should look like ultimately
//for (Mesh : meshes)
//{
//	/* Do whatever transformations here */
//
//	mesh->update();
//	mesh->set_modelview(camera.view_matrix);
//	mesh->transform();
//}
//  // NOTE: mesh->transform will transform all the lines of the gizmo as well!!
//	// eg:
//	// Mesh::transform()
//	// {
//	//    transform_mesh();
//	//    transform_gizmo();
//	// }
//	// Where transform_gizmo transforms all the vertices making up the
//	// gizmo's lines using Math3D::transform_point
void World::transform_meshes()
{
	for (Mesh*& mesh : meshes)
	{
		// Update the transforms of all models with this global transform
		glm::vec3 scale(1.0f);
		rot3 rotation(0.0f, x, 0.0f);
		glm::vec3 translation(0.0f, 0.0f, 0.0f);
		mesh->rotate(rotation);

		// Update the world transforms for each model
		mesh->update();

		// Update the positons of all lines and triangles in the scene
		transform_mesh(mesh);
		// TODO: Each mesh will have it's own individual gizmo. We'll have to transform those with a transform_gizmo function
		// Or have a transform member function for the gizmo. That'd be much cleaner I think
		// mesh->gizmo.transform();
	}
}

void World::transform_mesh(Mesh* mesh)
{
	glm::mat4 mv_mat = camera.view_matrix * light.transform;
	for (glm::vec4& point : light.direction_vector.points)
	{
		Math3D::transform_point(point, mv_mat);
	}
	lines_in_scene.push_back(light.direction_vector);

	Logger::info(LOG_CATEGORY_LIGHT, "Direction vector start (camera space): " + vec4_to_string(light.direction_vector.points[0]));
	Logger::info(LOG_CATEGORY_LIGHT, "Direction vector end (camera space): " + vec4_to_string(light.direction_vector.points[1]));
	Logger::info(LOG_CATEGORY_LIGHT, "Light transform matrix:\n" + mat4_to_string(light.transform));
	Logger::info(LOG_CATEGORY_LIGHT, "Camera view matrix:\n" + mat4_to_string(camera.view_matrix));
	Logger::info(LOG_CATEGORY_LIGHT, "Light model-view matrix:\n" + mat4_to_string(mv_mat));

	for (const Triangle& triangle : mesh->triangles)
	{
		/* Model space */
		// Initialize the new triangle
		Triangle transformed_triangle = triangle;

		// Transform the vertices by the model matrix
		for (glm::vec4& vertex : transformed_triangle.vertices)
		{
			Math3D::transform_point(vertex, mesh->transform); // TODO: make these into mesh class member functions?
		}	
		// Rotate the vertex normals by the model matrix
		for (glm::vec3& normal : transformed_triangle.normals)
		{
			Math3D::rotate_normal(normal, mesh->transform); // TODO: make these into mesh class member functions?
		}

		// This needs to happen after the mesh is transformed by the model matrix, but before it gets transformed by the camera matrix
		// Compute the lighting in world space (camera direction is never transformed by the view matrix)
		// Compute the face normal of the triangle
		glm::vec3 ab = glm::vec3(triangle.vertices[1] - triangle.vertices[0]);
		glm::vec3 ca = glm::vec3(triangle.vertices[2] - triangle.vertices[0]);
		glm::vec3 face_normal = glm::cross(ab, ca);
		face_normal = glm::normalize(face_normal);
		// Rotate the face normal
		Math3D::rotate_normal(face_normal, mesh->transform);
		transformed_triangle.face_normal = face_normal;
		// Using the face normal, assign a light intensity value to each face using the dot product
		glm::vec3 light_direction = light.direction;
		float intensity = -glm::dot(face_normal, light_direction);
		// Map intensity value from [-1, 1] to [0, 1]
		intensity = (intensity + 1.0f) * 0.5f;
		transformed_triangle.flat_value = intensity;

		// Compute the intensity for each vertex
		float v0_intensity = -glm::dot(transformed_triangle.normals[0], light_direction);
		float v1_intensity = -glm::dot(transformed_triangle.normals[1], light_direction);
		float v2_intensity = -glm::dot(transformed_triangle.normals[2], light_direction);
		// Map intensity values from [-1, 1] to [0, 1]
		v0_intensity = (v0_intensity + 1.0f) * 0.5f;
		v1_intensity = (v1_intensity + 1.0f) * 0.5f;
		v2_intensity = (v2_intensity + 1.0f) * 0.5f;
		transformed_triangle.gouraud[0] = v0_intensity;
		transformed_triangle.gouraud[1] = v1_intensity;
		transformed_triangle.gouraud[2] = v2_intensity;
		
		// Transform the vertices by the view matrix
		for (glm::vec4& vertex : transformed_triangle.vertices)
		{
			Math3D::transform_point(vertex, camera.view_matrix); // TODO: make these into mesh class member functions?
		}

		// Rotate the vertex normals by the view matrix
		for (glm::vec3& vertex_normal : transformed_triangle.normals)
		{
			Math3D::rotate_normal(vertex_normal, camera.view_matrix); // TODO: make these into mesh class member functions?
		}

		// Add the transformed triangle to the bin of triangles to be rendered
		triangles_in_scene.push_back(transformed_triangle);
	}

	// Rotate the light direction vector by the view matrix
	Math3D::rotate_normal(light.direction, camera.view_matrix);
}

// TODO: this should really be split into two functions. I think each mesh should have it's own individual gizmo, which it calls in the update
void World::transform_lines()
{
	// Transform the bases and origin by the modelview matrix
	for (Line3D& basis : gizmo.bases)
	{
		for (glm::vec4& point : basis.points)
		{
			Math3D::transform_point(point, modelview_matrix);
		}

		lines_in_scene.push_back(basis);
	}


	// TODO: Modelview for the light entity? Might not be needed
	//glm::mat4 mv_mat = camera.view_matrix * light.transform;


	//for (glm::vec4& point : light.direction_vector.points)
	//{
	//	Math3D::transform_point(point, mv_mat);
	//}
	//lines_in_scene.push_back(light.direction_vector);

	//Logger::info(LOG_CATEGORY_LIGHT, "Direction vector start (camera space): " + vec4_to_string(light.direction_vector.points[0]));
	//Logger::info(LOG_CATEGORY_LIGHT, "Direction vector end (camera space): " + vec4_to_string(light.direction_vector.points[1]));
	//Logger::info(LOG_CATEGORY_LIGHT, "Light transform matrix:\n" + mat4_to_string(light.transform));
	//Logger::info(LOG_CATEGORY_LIGHT, "Camera view matrix:\n" + mat4_to_string(camera.view_matrix));
	//Logger::info(LOG_CATEGORY_LIGHT, "Light model-view matrix:\n" + mat4_to_string(mv_mat));
}

void World::compute_face_normal(Triangle& triangle)
{
	glm::vec3 ab = glm::vec3(triangle.vertices[1] - triangle.vertices[0]);
	glm::vec3 ca = glm::vec3(triangle.vertices[2] - triangle.vertices[0]);
	glm::vec3 normal = glm::cross(ab, ca);
	normal = glm::normalize(normal);
	triangle.face_normal = normal;
}

void World::compute_light_intensity(Triangle& triangle)
{
	glm::vec3 light_direction = light.direction;
	// Compute the intensity for each face
	glm::vec3 face_normal = triangle.face_normal;
	float intensity = -glm::dot(face_normal, light_direction);
	triangle.flat_value = intensity;

	// Compute the intensity for each vertex
	float v0_intensity = -glm::dot(triangle.normals[0], light_direction);
	float v1_intensity = -glm::dot(triangle.normals[1], light_direction);
	float v2_intensity = -glm::dot(triangle.normals[2], light_direction);
	triangle.gouraud[0] = v0_intensity;
	triangle.gouraud[1] = v1_intensity;
	triangle.gouraud[2] = v2_intensity;
}
