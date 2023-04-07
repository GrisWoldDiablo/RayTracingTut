#include "Renderer.h"

#include <complex>
#include <dinput.h>
#include <execution>
#include <glm/gtc/epsilon.hpp>

#include "Scene.h"
#include "Camera.h"
#include "Ray.h"
#include "Utils.h"
#include "Walnut/Random.h"

Renderer::Renderer()
	: Bounces(2),
	LightDirection(-1.0f, -1.0f, -1.0f),
	BackColor(0.2f, 0.2f, 0.2) {}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (_finalImage)
	{
		if (_finalImage->GetWidth() == width && _finalImage->GetHeight() == height)
		{
			return;
		}

		_finalImage->Resize(width, height);
	}
	else
	{
		_finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	const uint32_t size = width * height;
	delete[] _imageData;
	_imageData = new uint32_t[size];

	delete[] _accumulationData;
	_accumulationData = new glm::vec4[size];

	_imageHorIter.resize(width);
	for (uint32_t i = 0; i < width; i++)
	{
		_imageHorIter[i] = i;
	}

	_imageVertIter.resize(height);
	for (uint32_t i = 0; i < height; i++)
	{
		_imageVertIter[i] = i;
	}
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	_activeScene = &scene;
	_activeCamera = &camera;

	if (_frameIndex == 1)
	{
		memset(_accumulationData, 0, _finalImage->GetWidth() * _finalImage->GetHeight() * sizeof(glm::vec4));
	}

	if (IsMultiThread)
	{
		std::for_each(std::execution::par, _imageVertIter.begin(), _imageVertIter.end(),
			[this](uint32_t y)
			{
				if (IsMultiThreadInner)
				{
					std::for_each(std::execution::par, _imageHorIter.begin(), _imageHorIter.end(),
						[this,y](uint32_t x)
						{
							const auto color = PerPixel(x, y);
							_accumulationData[x + y * _finalImage->GetWidth()] += color;

							glm::vec4 accumulatedColor = _accumulationData[x + y * _finalImage->GetWidth()];
							accumulatedColor /= static_cast<float>(_frameIndex);

							accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
							_imageData[x + y * _finalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
						});
				}
				else
				{
					for (uint32_t x = 0; x < _finalImage->GetWidth(); x++)
					{
						const auto color = PerPixel(x, y);
						_accumulationData[x + y * _finalImage->GetWidth()] += color;

						glm::vec4 accumulatedColor = _accumulationData[x + y * _finalImage->GetWidth()];
						accumulatedColor /= static_cast<float>(_frameIndex);

						accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
						_imageData[x + y * _finalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
					}
				}
			});
	}
	else
	{
		// render every pixel
		for (uint32_t y = 0; y < _finalImage->GetHeight(); y++)
		{
			for (uint32_t x = 0; x < _finalImage->GetWidth(); x++)
			{
				const auto color = PerPixel(x, y);
				_accumulationData[x + y * _finalImage->GetWidth()] += color;

				glm::vec4 accumulatedColor = _accumulationData[x + y * _finalImage->GetWidth()];
				accumulatedColor /= static_cast<float>(_frameIndex);

				accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				_imageData[x + y * _finalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
			}
		}
	}

	_finalImage->SetData(_imageData);
	if (_settings.ShouldAccumulate)
	{
		_frameIndex++;
	}
	else
	{
		_frameIndex = 1;
	}
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

		const Sphere& sphere = _activeScene->Spheres[payload.ObjectIndex];
		if (sphere.MaterialIndex > _activeScene->Materials.size() - 1)
		{
			break;
		}

		const Material& material = _activeScene->Materials[sphere.MaterialIndex];

		auto sphereColor = material.Albedo;
		sphereColor *= lightIntensity;

		color += sphereColor * multiplier;
		multiplier *= 0.5f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction,
			payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}

	return glm::vec4(color, 1.0f);
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
