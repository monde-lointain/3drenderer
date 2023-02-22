#include "Entity.h"

#include "../Math/Math3D.h"

Entity::Entity(glm::vec3 scale_, rot3 rotation_, glm::vec3 translation_)
{
	scale = scale_;
	rotation = rotation_;
	translation = translation_;
	transform = Math3D::create_world_matrix(scale, rotation, translation);
}

void Entity::update()
{
	transform = Math3D::create_world_matrix(scale, rotation, translation);
}
