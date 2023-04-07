#include "Utils.h"

uint32_t Utils::ConvertToRGBA(const glm::vec4& color)
{
	const auto r = static_cast<uint8_t>(color.r * 255.0f);
	const auto g = static_cast<uint8_t>(color.g * 255.0f);
	const auto b = static_cast<uint8_t>(color.b * 255.0f);
	const auto a = static_cast<uint8_t>(color.a * 255.0f);
	const uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
	return result;
}

uint32_t Utils::ConvertToRGBA(const glm::vec3& color)
{
	const auto r = static_cast<uint8_t>(color.r * 255.0f);
	const auto g = static_cast<uint8_t>(color.g * 255.0f);
	const auto b = static_cast<uint8_t>(color.b * 255.0f);
	const uint32_t result = (255 << 24) | (b << 16) | (g << 8) | r;
	return result;
}
