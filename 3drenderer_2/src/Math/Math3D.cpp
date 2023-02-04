#include "Math3D.h"

#include "../Mesh/Mesh.h"
#include "../Renderer/Camera.h"
#include "../Renderer/Gizmo.h"
#include "../Renderer/Viewport.h"

glm::mat4 Math3D::create_projection_matrix(const Camera& camera)
{
	glm::mat4 projection_matrix = glm::perspective(
		glm::radians(camera.fov), camera.aspect, camera.znear, camera.zfar);
	return projection_matrix;
}

glm::mat4 Math3D::create_world_matrix(
	glm::vec3 scale, glm::vec3 rotation, glm::vec3 translation)
{
	glm::mat4 rotation_x_matrix = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotation_y_matrix = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotation_z_matrix = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	// NOTE: Order matters here! For a left-handed coordinate system these would
	// happen in the opposite order
	glm::mat4 rotation_matrix =
		rotation_z_matrix * rotation_y_matrix * rotation_x_matrix;
	glm::mat4 world_matrix =
		translation_matrix * rotation_matrix * scale_matrix;
	return world_matrix;
}

void Math3D::transform_point(glm::vec4& point, const glm::mat4& world_matrix,
	const glm::mat4& view_matrix)
{
	// Transform the point from model space to world space
	point = glm::vec4(world_matrix * point);
	// Transform the point from world space to camera space
	point = glm::vec4(view_matrix * point);
}

void Math3D::project_point(glm::vec4& point, const glm::mat4& projection_matrix,
	const Viewport& viewport)
{
	// Transform the point from camera space to clip space
	point = glm::vec4(projection_matrix * point);

	// Normalize the point by 
	if (point.w != 0.0f)
	{
		point.x /= point.w;
		point.y /= point.w;
		point.z /= point.w;
	}

	// Scale into the view
	point.x *= ((float)viewport.width * 0.5f);
	point.y *= ((float)viewport.height * 0.5f);

	// Translate the projected points to the middle of the screen
	point.x += ((float)viewport.width * 0.5f);
	point.y += ((float)viewport.height * 0.5f);
}

int Math3D::orient2d_i(const vec2i& a, const vec2i& b, const vec2i& c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

float Math3D::orient2d_f(
	const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
