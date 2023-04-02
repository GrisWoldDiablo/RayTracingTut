#include "Renderer.h"

#include <complex>
#include <iostream>

#include "Camera.h"
#include "Ray.h"

namespace Utils
{
	uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		const auto r = static_cast<uint8_t>(color.r * 255.0f);
		const auto g = static_cast<uint8_t>(color.g * 255.0f);
		const auto b = static_cast<uint8_t>(color.b * 255.0f);
		const auto a = static_cast<uint8_t>(color.a * 255.0f);
		const uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

Renderer::Renderer()
	: Radius(0.5f),
	SpherePosition(0.0f, 0.0f, 0.0f),
	LightPosition(1.0f, 1.0f, 1.0f),
	SphereColor(1.0f, 0.f, 1.0f, 1.0f),
	BackColor(0.5f, 0.5f, 0.5f, 1.0f) {}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (_finalImage)
	{
		_finalImage->Resize(width, height);
	}
	else
	{
		_finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	uint32_t size = width * height;
	delete[] _imageData;
	_imageData = new uint32_t[size];
}

void Renderer::Render(const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	// render every pixel
	for (uint32_t y = 0; y < _finalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < _finalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * _finalImage->GetWidth()];
			_imageData[x + y * _finalImage->GetWidth()] = Utils::ConvertToRGBA(TraceRay(ray));
		}
	}

	_finalImage->SetData(_imageData);
}

glm::vec4 Renderer::TraceRay(const Ray& ray)
{
	glm::vec4 color = SphereColor;
	glm::vec3 lightDir = glm::normalize(LightPosition - SpherePosition);

	// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0

	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	//float a = rayDirection.x * rayDirection.x
	//		+ rayDirection.y * rayDirection.y
	//		+ rayDirection.z * rayDirection.z;

	const glm::vec3 rayOrigin = ray.Origin - SpherePosition;
	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(rayOrigin, ray.Direction);
	float c = glm::dot(rayOrigin, rayOrigin) - Radius * Radius;

	// Quadratic formula discriminant
	// b^2 - 4ac
	// (-b +- sqrt(discriminant)) / (2.0f * a)

	const float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
	{
		return BackColor;
	}

	float closesT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);

	glm::vec3 hitPosition = rayOrigin + ray.Direction * closesT;
	glm::vec3 normal = glm::normalize(hitPosition);
	float lightIntensity = glm::max(0.0f, glm::dot(normal, lightDir));

	return color * lightIntensity;
}
