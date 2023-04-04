#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Sphere
{
	float Radius = 0.5f;
	glm::vec3 Position{0.0f};
	
	glm::vec3 Albedo{1.0f};
};
