#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "../Entity/Entity.h"
#include "../Math/Rotator.h"

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

enum EProjectionMode
{
	PERSPECTIVE  = 1 << 0,
	ORTHOGRAPHIC = 1 << 1,
};

typedef int MovementState;
typedef int ProjectionMode;

struct Camera : public Entity
{
	Camera(glm::vec3 position = glm::vec3(0.0f), rot3 rotation = rot3(0.0f));

	void update();
	void process_mouse_movement();
	void update_camera_vectors();
	void update_position();

	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 move_direction = direction;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

	float speed = 0.1f;
	const float SPEED_INC = 0.025f;
	const float MAX_SPEED = 10.0f;
	const float MIN_SPEED = 0.001f;

	float fov = 0.0f; // in degrees
	float aspect_x = 0.0f;
	float aspect_y = 0.0f;
	float aspect = 0.0f;
	float znear = 0.0f;
	float zfar = 0.0f;
	float ortho_dist = 20.0f;

	glm::mat4 view_matrix = glm::mat4(1.0f);
	glm::mat4 projection_matrix = glm::mat4(1.0f);
	glm::mat4 vp_matrix = glm::mat4(1.0f);
	ProjectionMode projection_mode = PERSPECTIVE;

	float mouse_sensitivity = 0.15f;
	bool window_clicked = false;
	bool mouse_has_position = false;

	EInputMode input_mode = MOUSE_INPUT_DISABLED;
	MovementState move_state = NOT_MOVING;

	void set_move_state(EMovementState state, bool set);
	void set_view();
	void set_projection(ProjectionMode mode);
};