#pragma once

#include "../Math/Rotator.h"
#include <glm/vec3.hpp>

// Bitmask for combining movement directions together into a single int
enum EMovementState
{
	NOT_MOVING = 0,
	FORWARD    = 1 << 0,
	BACKWARD   = 1 << 1,
	RIGHT      = 1 << 2,
	LEFT       = 1 << 3,
	UP         = 1 << 4,
	DOWN       = 1 << 5,
};

// Bitmask for controlling which types of inputs are allowed
enum EInputMode
{
	INPUT_ENABLED           = 0,
	MOUSE_INPUT_DISABLED    = 1 << 0,
	KEYBOARD_INPUT_DISABLED = 1 << 1,
	INPUT_DISABLED          = MOUSE_INPUT_DISABLED | KEYBOARD_INPUT_DISABLED
};

typedef int MovementState;

struct Camera
{
	Camera();
	~Camera() = default;

	void update();

	void process_mouse_movement();
	void update_camera_vectors();
	void update_position();

	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;
	float speed;
	rot3 rotation; // in degrees
	float fov; // in degrees
	float aspect;
	float znear;
	float zfar;
	float mouse_sensitivity;
	MovementState move_state;
	glm::vec3 move_direction;
	EInputMode input_mode;

	void set_move_state(EMovementState state, bool set);
};