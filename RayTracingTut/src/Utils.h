#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Utils
{
public:
	static uint32_t ConvertToRGBA(const glm::vec4& color);
	static uint32_t ConvertToRGBA(const glm::vec3& color);
};
