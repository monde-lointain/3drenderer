#include "Camera.h"

Camera::Camera()
{
	position = glm::vec3(0.0f);
	fov = 0.0f;
	aspect = 0.0f;
	znear = 0.0f;
	zfar = 0.0f;
}
