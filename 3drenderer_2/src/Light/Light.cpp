#include "Light.h"

#include "../Math/Math3D.h"
#include "../Utils/Colors.h"
#include "../Logger/Logger.h"
#include "../Utils/string_ops.h"
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

Light::Light(glm::vec3 translation, rot3 rotation, float intensity)
{
	this->translation = translation;
	this->rotation = rotation;
	this->intensity = intensity;
	// Compute the direction and rotation vector
	update();
	this->direction_vector.color = Colors::WHITE;
}

void Light::update()
{
	// Create the rotation matrix
	glm::mat4 rotation_x_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotation_y_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.yaw), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 rotation_z_matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.roll), glm::vec3(0.0f, 0.0f, -1.0f));
	glm::mat4 rotation_matrix = rotation_x_matrix * rotation_y_matrix * rotation_z_matrix;

	// Set the points on the direction vector here. We do NOT transform them yet.
	// They get transformed by the modelview matrix slightly after being updated
	glm::vec4 normal = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	direction_vector.points[0] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	direction_vector.points[1] = normal;

	// Rotate the normal by the rotation matrix
	normal = rotation_matrix * normal;
	// Compute the direction vector
	direction = glm::vec3(normal);
	//// Set the start and end points of the direction vector
	//glm::vec4 start = glm::vec4(translation, 1.0f);
	//glm::vec4 end = start + (glm::vec4(direction, 0.0f) * 10.0f);

	transform = Math3D::create_world_matrix(glm::vec3(1.0f), rotation, translation);

	Logger::info(LOG_CATEGORY_LIGHT, "Light position: " + vec3_to_string(translation));
	Logger::info(LOG_CATEGORY_LIGHT, "Light rotation: " + rotation.to_string());
	Logger::info(LOG_CATEGORY_LIGHT, "Light direction: " + vec3_to_string(direction));
	Logger::info(LOG_CATEGORY_LIGHT, "Direction vector start (local space): " + vec4_to_string(direction_vector.points[0]));
	Logger::info(LOG_CATEGORY_LIGHT, "Direction vector end (local space): " + vec4_to_string(direction_vector.points[1]));
}
