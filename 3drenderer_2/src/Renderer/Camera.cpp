#include "Camera.h"

#include "../Logger/Logger.h"
#include "../Utils/3d_types.h"
#include "../Utils/string_ops.h"
#include <SDL.h>
#include <glm/glm.hpp>

Camera::Camera()
{
	position = glm::vec3(0.0f);
	direction = glm::vec3(0.0f, 0.0f, 0.0f);
	world_up = glm::vec3(0.0f, 1.0f, 0.0f);
	up = world_up;
	right = glm::vec3(1.0f, 0.0f, 0.0f);
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
	input_mode = MOUSE_INPUT_DISABLED;
}

void Camera::update()
{
	process_mouse_movement();
	update_position();

	// Logging
	Logger::info(LOG_CATEGORY_CAMERA, "Camera position: " + vec3_to_string(position));
	Logger::info(LOG_CATEGORY_CAMERA, "Camera rotation: " + rotation.to_string());
}

void Camera::process_mouse_movement()
{
	if (input_mode & MOUSE_INPUT_DISABLED)
	{
		update_camera_vectors();
		return;
	}

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

void Camera::update_position()
{
	if (input_mode & KEYBOARD_INPUT_DISABLED)
	{
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

	// Our velocity is zero if we press two buttons going in opposite directions
	// (left+right/up+down/etc.) and aren't pressing any other buttons. This
	// will cause an error in the direction computations so we need to return
	// early here
	if (move_dir == glm::vec3(0.0f))
	{
		return;
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
