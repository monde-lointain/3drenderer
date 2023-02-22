#include "Camera.h"

#include "../Graphics/Graphics.h"
#include "../Logger/Logger.h"
#include "../Utils/3d_types.h"
#include "../Utils/string_ops.h"
#include <glm/gtc/matrix_transform.hpp>

#ifdef _MSC_VER // Windows
#include <SDL.h>
#else // Linux
#include <SDL2/SDL.h>
#endif

Camera::Camera(glm::vec3 position, rot3 rotation)
	: Entity(glm::vec3(1.0f), rotation, position)
{
	this->direction = glm::vec3(0.0f, 0.0f, 0.0f);
	this->move_direction = direction;
	this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
	this->up = world_up;
	this->right = glm::vec3(1.0f, 0.0f, 0.0f);
	this->speed = 0.1f;
	this->move_state = NOT_MOVING;
	this->fov = 0.0f;
	this->aspect = 0.0f;
	this->znear = 0.0f;
	this->zfar = 0.0f;
	this->mouse_sensitivity = 0.15f;
	this->window_clicked = false;
	this->mouse_has_position = false;
	this->input_mode = MOUSE_INPUT_DISABLED;
}

void Camera::update()
{
	process_mouse_movement();
	update_position();

	// Logging
	Logger::info(LOG_CATEGORY_CAMERA, "Camera position: " + vec3_to_string(translation));
	Logger::info(LOG_CATEGORY_CAMERA, "Camera rotation: " + rotation.to_string());
	Logger::info(LOG_CATEGORY_CAMERA, "Camera speed: " + std::to_string(speed));
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

	// Set the mouse focus manually when the window is first clicked
	if (window_clicked && !mouse_has_position)
	{
		SDL_GetRelativeMouseState(&x, &y);
		x = 0;
		y = 0;
		mouse_has_position = true;
	}

	// Adjust to be relative to the center of the window
	float xoffset = (float)x * mouse_sensitivity;
	float yoffset = (float)y * mouse_sensitivity;

	rotation.yaw += xoffset;
	// We're drawing from the bottom up, so we need to flip this, since SDL
	// draws from the top down by default
	rotation.pitch -= yoffset;

	// Clamp the pitch so we can't rotate the camera backwards
	rotation.pitch = glm::clamp(rotation.pitch, -89.0f, 89.0f);

	//Logger::info(LOG_CATEGORY_CAMERA, "mouse x: " + std::to_string(x));
	//Logger::info(LOG_CATEGORY_CAMERA, "mouse y: " + std::to_string(y));
	//Logger::info(LOG_CATEGORY_CAMERA, "x offset: " + std::to_string(xoffset));
	//Logger::info(LOG_CATEGORY_CAMERA, "y offset: " + std::to_string(yoffset));

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

	if (move_state & FORWARD)
	{
		move_dir += direction;
	}
	if (move_state & BACKWARD)
	{
		move_dir -= direction;
	}
	if (move_state & RIGHT)
	{
		move_dir += right;
	}
	if (move_state & LEFT)
	{
		move_dir -= right;
	}
	if (move_state & UP)
	{
		move_dir += world_up;
	}
	if (move_state & DOWN)
	{
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

	translation += move_direction * speed;
}

void Camera::set_move_state(EMovementState state, bool set)
{
	if (set)
	{
		move_state |= state;
	}
	else
	{
		move_state &= ~state;
	}
}

void Camera::set_view()
{
	glm::vec3 target = translation + direction;
	view_matrix = glm::lookAt(translation, target, world_up);
	vp_matrix = projection_matrix * view_matrix;
}

void Camera::set_projection(ProjectionMode mode)
{
	// Set perspective
	if (mode & PERSPECTIVE)
	{
		projection_mode |= PERSPECTIVE;
		projection_matrix = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}
	if (mode & ORTHOGRAPHIC)
	{
		projection_mode |= ORTHOGRAPHIC;
		projection_matrix = glm::ortho(-ortho_dist * aspect, ortho_dist * aspect, -ortho_dist, ortho_dist, znear, zfar);
	}
}
