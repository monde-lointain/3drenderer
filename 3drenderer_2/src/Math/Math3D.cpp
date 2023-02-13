#include "Math3D.h"

#include "../Mesh/Mesh.h"
#include "../Renderer/Camera.h"
#include "../Renderer/Gizmo.h"
#include "../Renderer/Viewport.h"
#include "../Utils/math_helpers.h"
#include <tracy/tracy/Tracy.hpp>

glm::mat4 Math3D::create_projection_matrix(const Camera& camera)
{
	glm::mat4 projection_matrix = glm::perspective(
		glm::radians(camera.fov), camera.aspect, camera.znear, camera.zfar);
	return projection_matrix;
}

glm::mat4 Math3D::create_world_matrix(
	glm::vec3 scale, glm::vec3 rotation, glm::vec3 translation)
{
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_x_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotation_y_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotation_z_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);
	// NOTE: Order matters here! For a left-handed coordinate system these would
	// happen in the opposite order
	glm::mat4 rotation_matrix = rotation_x_matrix * rotation_y_matrix * rotation_z_matrix;
	glm::mat4 world_matrix = translation_matrix * rotation_matrix * scale_matrix;
	return world_matrix;
}

void Math3D::transform_point(glm::vec4& point, const glm::mat4& modelview_matrix)
{
	// Transform the point from model space to camera space
	point = glm::vec4(modelview_matrix * point);
}

void Math3D::rotate_normal(glm::vec3& normal, const glm::mat4& modelview_matrix)
{
	// Transform the normals by the inverse-transpose of the model-view matrix
	glm::mat3 inverse = glm::inverse(glm::mat3(modelview_matrix));
	glm::mat3 normal_matrix = glm::transpose(inverse);
	normal = glm::vec3(normal_matrix * normal);
}

void Math3D::project(glm::vec4& point, const glm::mat4& projection_matrix)
{
	point = glm::vec4(projection_matrix * point);
}

void Math3D::to_ndc(glm::vec4& point, float& one_over_w)
{
	point.x *= one_over_w;
	point.y *= one_over_w;
	point.z *= one_over_w;
}

void Math3D::to_screen_space(glm::vec4& point, const Viewport& viewport, const Camera& camera)
{
	// Transform the point from clip space to screen space
	point.x = (point.x + 1.0f) * (float)viewport.width * 0.5f;
	point.y = (point.y + 1.0f) * (float)viewport.height * 0.5f;

	// Normalize the z coordinate [0, 1]
	//point.z = (point.z * camera.zfar + camera.zfar - point.z * camera.znear + camera.znear) * 0.5f;
	point.z = (point.z + 1.0f) * 0.5f;
}

// NOTE: This is only called for Gizmo points only! Triangles have a different
// version of this in Renderer::project_triangle. I should really move that
// function over to this namespace honestly
void Math3D::project_point(glm::vec4& point, const glm::mat4& projection_matrix,
	const Viewport& viewport, const Camera& camera)
{
	// Transform the point from camera space to clip space
	project(point, projection_matrix);

	float one_over_w = is_nearly_zero(point.w) ? 1.0f : 1.0f / point.w;

	// Perform the perspective divide
	to_ndc(point, one_over_w);

	to_screen_space(point, viewport, camera);
}

int Math3D::orient2d_i(const vec2i& a, const vec2i& b, const vec2i& c)
{
	int signed_area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	// TODO: Flip the sign of the signed area based on the winding order of
	// the triangle. CW would be *= -1.0f and CCW *=  1.0f.
	return signed_area;
}

float Math3D::orient2d_f(
	const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	float signed_area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	// See comment for orient2d_i
	return signed_area;
}
