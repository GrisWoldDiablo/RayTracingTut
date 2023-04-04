#include "Renderer.h"

#include <complex>
#include <iostream>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Scene.h"
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
	: LightPosition(1.0f, 1.0f, 1.0f),
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

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	// render every pixel
	for (uint32_t y = 0; y < _finalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < _finalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * _finalImage->GetWidth()];
			_imageData[x + y * _finalImage->GetWidth()] = Utils::ConvertToRGBA(TraceRay(scene, ray));
		}
	}

	_finalImage->SetData(_imageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray) const
{
	if (scene.Spheres.empty())
	{
		return BackColor;
	}

	const Sphere* closestSphere = nullptr;
	float closestHit = FLT_MAX;
	for (const auto& sphere : scene.Spheres)
	{
		if (IntersectSphere(ray, sphere, closestHit))
		{
			closestSphere = &sphere;
		}
	}

	if (closestSphere == nullptr)
	{
		return BackColor;
	}

	return DrawSphere(ray, *closestSphere, closestHit);
}

bool Renderer::IntersectSphere(const Ray& ray, const Sphere& sphere, float& closestHit) const
{
	const glm::vec3 rayOrigin = ray.Origin - sphere.Position;
	const float a = glm::dot(ray.Direction, ray.Direction);
	const float b = 2.0f * glm::dot(rayOrigin, ray.Direction);
	const float c = glm::dot(rayOrigin, rayOrigin) - sphere.Radius * sphere.Radius;

	const float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
	{
		return false;
	}

	// Get the closest point on the sphere from the camera
	float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	// Get the point the second intersection point after closest point.
	float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);

	// If the closest point is negative it means it is behind the camera.
	if (closestT < 0.0f)
	{
		// If the second point is also negative it means it is also behind the camera.
		if (t0 < 0.0f)
		{
			return false;
		}
		
		// Else it means we are inside the sphere so we will use the second point.
		closestT = t0;
	}

	if (closestT < closestHit)
	{
		closestHit = closestT;
		return true;
	}

	return false;
}

glm::vec4 Renderer::DrawSphere(const Ray& ray, const Sphere& sphere, float hitPoint) const
{
	const glm::vec3 rayOrigin = ray.Origin - sphere.Position;
	const glm::vec3 lightDir = glm::normalize(LightPosition - sphere.Position);

	const glm::vec3 hitPosition = rayOrigin + ray.Direction * hitPoint;
	const glm::vec3 normal = glm::normalize(hitPosition);
	const float lightIntensity = glm::max(0.0f, glm::dot(normal, lightDir));

	return glm::vec4(sphere.Albedo * lightIntensity, 1.0f);
}
