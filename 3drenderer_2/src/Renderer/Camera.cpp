#include "Camera.h"

#include "../Utils/3d_types.h"
#include <SDL.h>
#include <glm/glm.hpp>

Camera::Camera()
{
	position = glm::vec3(0.0f);
	direction = glm::vec3(0.0f, 0.0f, 0.0f);
	world_up = glm::vec3(0.0f, 1.0f, 0.0f);
	speed = 0.1f;
	move_state = NOT_MOVING;
	rotation.roll = 0.0f;
	rotation.yaw = -90.0f;
	rotation.pitch = 0.0f;
	fov = 0.0f;
	aspect = 0.0f;
	znear = 0.0f;
	zfar = 0.0f;
	mouse_sensitivity = 0.1f;
}

void Camera::process_mouse_movement()
{
	// Get the x and y coordinates of the mouse
	int x, y;
	SDL_GetRelativeMouseState(&x, &y);
	float xoffset = (float)x * mouse_sensitivity;
	float yoffset = (float)y * mouse_sensitivity;

	rotation.yaw += xoffset;
	// We're drawing from the bottom up, so we need to flip this, since SDL
	// draws from the top down by default
	rotation.pitch -= yoffset;

	// Clamp the pitch so we can't rotate the camera backwards
	rotation.pitch = glm::clamp(rotation.pitch, -89.0f, 89.0f);

	update_camera_vectors();
}

void Camera::update_camera_vectors()
{
	float yaw = glm::radians(rotation.yaw);
	float pitch = glm::radians(rotation.pitch);
	glm::vec3 front(0.0f);
	front.x = cos(yaw) * cos(pitch);
	front.y = sin(pitch);
	front.z = sin(yaw) * cos(pitch);
	direction = glm::normalize(front);

	right = glm::normalize(glm::cross(direction, world_up));
	up = glm::normalize(glm::cross(right, direction));
}

void Camera::update_camera_position()
{
	if (move_state == NOT_MOVING) {
		return;
	}

	glm::vec3 move_dir(0.0f);

	if (move_state & FORWARD) {
		move_dir += direction;
	}
	if (move_state & BACKWARD) {
		move_dir -= direction;
	}
	if (move_state & RIGHT) {
		move_dir += right;
	}
	if (move_state & LEFT) {
		move_dir -= right;
	}
	if (move_state & UP) {
		move_dir += world_up;
	}
	if (move_state & DOWN) {
		move_dir -= world_up;
	}

	move_direction = glm::normalize(move_dir);

	position += move_direction * speed;
}

void Camera::set_move_state(EMovementState state, bool set)
{
	if (set) {
		move_state |= state;
	}
	else {
		move_state &= ~state;
	}
}
