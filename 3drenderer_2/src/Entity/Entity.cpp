#include "Entity.h"

#include "../Math/Math3D.h"

Entity::Entity(glm::vec3 scale, rot3 rotation, glm::vec3 translation)
{
	this->scale = scale;
	this->rotation = rotation;
	this->translation = translation;
	transform = Math3D::create_world_matrix(scale, rotation, translation);
}

void Entity::update()
{
	transform = Math3D::create_world_matrix(scale, rotation, translation);
}
