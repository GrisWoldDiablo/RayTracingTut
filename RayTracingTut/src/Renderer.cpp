#include "Renderer.h"

#include <complex>
#include <iostream>
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
	: Bounces(2),
	LightDirection(-1.0f, -1.0f, -1.0f),
	BackColor(0.2f, 0.2f, 0.2) {}

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
	_activeScene = &scene;
	_activeCamera = &camera;

	// render every pixel
	for (uint32_t y = 0; y < _finalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < _finalImage->GetWidth(); x++)
		{
			auto color = PerPixel(x, y);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			_imageData[x + y * _finalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	_finalImage->SetData(_imageData);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) const
{
	int closestSphere = -1;
	float closestHit = std::numeric_limits<float>::max();
	for (size_t i = 0; i < _activeScene->Spheres.size(); i++)
	{
		const auto& sphere = _activeScene->Spheres[i];
		if (IntersectSphere(ray, sphere, closestHit))
		{
			closestSphere = static_cast<int>(i);
		}
	}

	if (closestSphere < 0)
	{
		return Miss(ray);
	}

	return ClosestHit(ray, closestHit, closestSphere);

	//return DrawSphere(ray, *closestSphere, closestHit);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) const
{
	Ray ray;
	ray.Origin = _activeCamera->GetPosition();
	ray.Direction = _activeCamera->GetRayDirections()[x + y * _finalImage->GetWidth()];

	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	for (int i = 0; i < Bounces; ++i)
	{
		HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = BackColor;
			color += skyColor * multiplier;
			break;
		}

		const glm::vec3 lightDir = glm::normalize(LightDirection);
		float lightIntensity = glm::max(0.0f, glm::dot(payload.WorldNormal, -lightDir));

		const Sphere& closestSphere = _activeScene->Spheres[payload.ObjectIndex];
		auto sphereColor = closestSphere.Albedo;
		sphereColor *= lightIntensity;

		color += sphereColor * multiplier;
		multiplier *= 0.7f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal);
	}

	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex) const
{
	HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = _activeScene->Spheres[objectIndex];

	const glm::vec3 rayOrigin = ray.Origin - closestSphere.Position;

	payload.WorldPosition = rayOrigin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);
	payload.WorldPosition += closestSphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray) const
{
	HitPayload payload{};
	payload.HitDistance = -1.0f;
	return payload;
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
