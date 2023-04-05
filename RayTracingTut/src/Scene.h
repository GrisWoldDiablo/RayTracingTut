#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Material
{
	glm::vec3 Albedo{1.0f};
	float Roughness = 1.0f;
	float Metallic = 0.0f;
};

struct Sphere
{
	float Radius = 0.5f;
	glm::vec3 Position{0.0f};
	int MaterialIndex = 0;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};
