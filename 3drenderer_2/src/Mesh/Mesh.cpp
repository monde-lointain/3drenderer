#include "Mesh.h"

#define FAST_OBJ_IMPLEMENTATION
#include "Triangle.h"
#include "../Utils/Colors.h"
#include "../Utils/debug_helpers.h"
#include <fast_obj.h>
#include <SDL_image.h>
#include <iostream>

Mesh::Mesh(glm::vec3 scale, rot3 rotation, glm::vec3 translation)
	: Entity(scale, rotation, translation)
{
}

void Mesh::load_from_obj(const char* filename)
{
	fastObjMesh* fast_mesh = fast_obj_read(filename);

	if (!fast_mesh)
	{
		std::cerr << "Failed to load " << filename << ".\n";
		return;
	}

	// Allocate memory for the texture pointer array
	int num_materials = fast_mesh->material_count;
	textures.resize(num_materials * sizeof(Texture*));

	// For each mesh
	for (uint32 i = 0; i < fast_mesh->object_count; i++)
	{
		fastObjGroup object = fast_mesh->objects[i];
		int idx = 0;

		// For each triangle
		for (uint32 j = 0; j < object.face_count; j++)
		{
			Triangle triangle;

			// Load the texture from the material.
			int material_index = fast_mesh->face_materials[object.face_offset + j];

			// Only load if the texture is not already loaded
			if (!textures[material_index])
			{
				char* material_filename = fast_mesh->materials[material_index].map_Kd.path;
				if (material_filename)
				{
					std::shared_ptr<Texture> texture = load_texture(material_filename);
					textures[material_index] = texture;
				}
				else
				{
					std::cerr << "No textures found for " << filename << ".\n";
				}
			}

			triangle.texture = textures[material_index];

			for (uint32 k = 0; k < 3; k++)
			{
				fastObjIndex index = fast_mesh->indices[object.index_offset + idx];

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
				triangle.normals[k] = glm::vec3(
					fast_mesh->normals[3 * index.n + 0],
					fast_mesh->normals[3 * index.n + 1],
					fast_mesh->normals[3 * index.n + 2]
				);

				idx++;
			}

			triangle.color = Colors::WHITE;
			triangles.push_back(triangle);
		}
	}

	std::cout << "Loaded " << filename << "\n";

	// Destroy the fastobj mesh once we've imported it
	fast_obj_destroy(fast_mesh);
}

std::shared_ptr<Texture> Mesh::load_texture(const char* filename)
{
	// Allocate the Texture object
	std::shared_ptr<Texture> texture = std::make_unique<Texture>();

	// Load the image using SDL_image
	SDL_Surface* surface = IMG_Load(filename);

	if (!surface)
	{
		std::cerr << "Failed to load " << filename << ".\n";
		return nullptr;
	}

	// Convert to the pixel format of the renderer
	surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);

	// Copy the pixels of the surface over to our struct
	texture->width = surface->w;
	texture->height = surface->h;
	texture->pixels = std::make_unique<uint32[]>(surface->w * surface->h);
	memcpy(texture->pixels.get(), surface->pixels, surface->w * surface->h * sizeof(uint32));

	// Free the created surface now that we're done with ti
	SDL_FreeSurface(surface);

	return texture;
}

void Mesh::rotate(rot3 amount)
{
	rotation.pitch += amount.pitch; // x
	rotation.yaw += amount.yaw; // y
	rotation.roll += amount.roll; // z
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
