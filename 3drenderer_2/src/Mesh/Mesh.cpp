#include "Mesh.h"

#define FAST_OBJ_IMPLEMENTATION
#include "Triangle.h"
#include "../Misc/Colors.h"
#include "../Misc/debug_helpers.h"
#include <fast_obj.h>
#include <iostream>

Mesh::Mesh()
{
	rotation = glm::vec3(0.0f);
	scale = glm::vec3(1.0f);
	translation = glm::vec3(0.0f);
	texture = new Texture;
}

Mesh::~Mesh()
{
	if (texture)
	{
		texture->destroy();
		delete texture;
	}
}

void Mesh::load_from_obj(const char* filename)
{
	fastObjMesh* fast_mesh = fast_obj_read(filename);

	if (!fast_mesh)
	{
		std::cerr << "Failed to load " << filename << ".\n";
		return;
	}

	// Find how many groups of faces there are in the mesh
	for (uint32 i = 0; i < fast_mesh->group_count; i++)
	{
		fastObjGroup group = fast_mesh->groups[i];
		int idx = 0;

		// Get the vertex, normal and texture coordinate information for each
		// triangle
		for (uint32 j = 0; j < group.face_count; j++)
		{
			Triangle triangle;

			for (uint32 k = 0; k < 3; k++)
			{
				fastObjIndex index = fast_mesh->indices[group.index_offset + idx];

				triangle.vertices[k] = glm::vec4(
					fast_mesh->positions[3 * index.p + 0],
					fast_mesh->positions[3 * index.p + 1],
					fast_mesh->positions[3 * index.p + 2],
					1.0f
				);
				triangle.texcoords[k] = {
					fast_mesh->texcoords[2 * index.t + 0],
					fast_mesh->texcoords[2 * index.t + 1]
				};
				triangle.normals[k] = glm::vec4(
					fast_mesh->normals[3 * index.n + 0],
					fast_mesh->normals[3 * index.n + 1],
					fast_mesh->normals[3 * index.n + 2],
					1.0f
				);

				idx++;
			}

			triangle.color = Colors::WHITE;
			triangles.push_back(triangle);
		}
	}

	// Destroy the fastobj mesh once we've imported it
	fast_obj_destroy(fast_mesh);
}

void Mesh::load_texture(const char* filename)
{
	texture->surface = IMG_Load(filename);

	if (!texture->surface) {
		std::cerr << "Failed to load " << filename << ".\n";
		set_texture_on_triangles(nullptr);
		return;
	}

	// Convert to the pixel format of the renderer
	texture->surface =
		SDL_ConvertSurfaceFormat(texture->surface, SDL_PIXELFORMAT_BGRA8888, 0);

	texture->width = texture->surface->w;
	texture->height = texture->surface->h;
	texture->pixels = (uint32*)texture->surface->pixels;

	// NOTE: ugly
	set_texture_on_triangles(texture);
}

void Mesh::set_texture_on_triangles(Texture* texture_to_set)
{
	for (Triangle& triangle : triangles)
	{
		triangle.texture = texture_to_set;
	}
}

void Mesh::rotate(float x, float y, float z)
{
	rotation.x += x;
	rotation.y += y;
	rotation.z += z;
}

void Mesh::mesh_scale(float x, float y, float z)
{
	scale.x += x;
	scale.y += y;
	scale.z += z;
}

int Mesh::num_triangles()
{
	return (int)triangles.size();
}

Mesh* create_mesh(const char* filename)
{
	Mesh* mesh = new Mesh;
	mesh->load_from_obj(filename);
	if (!mesh)
	{
		return nullptr;
	}
	return mesh;
}
