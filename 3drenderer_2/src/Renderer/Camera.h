#pragma once

#include "../Entity/Entity.h"
#include "../Math/Rotator.h"
#include <glm/mat4x4.hpp>
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
	~Camera() = default;

	void update();
	void process_mouse_movement();
	void update_camera_vectors();
	void update_position();

	glm::vec3 direction;
	glm::vec3 move_direction;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	float speed;
	const float SPEED_INC = 0.025f;
	const float MAX_SPEED = 10.0f;
	const float MIN_SPEED = 0.001f;

	float fov; // in degrees
	float aspect_x;
	float aspect_y;
	float aspect;
	float znear;
	float zfar;
	float ortho_dist = 20.0f;

	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;
	glm::mat4 vp_matrix;
	ProjectionMode projection_mode;

	float mouse_sensitivity;
	bool window_clicked;
	bool mouse_has_position;

	EInputMode input_mode;
	MovementState move_state;

	void set_move_state(EMovementState state, bool set);
	void set_view();
	void set_projection(ProjectionMode mode);
};