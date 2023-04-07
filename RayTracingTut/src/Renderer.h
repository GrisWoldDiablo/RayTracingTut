#pragma once


#include "Walnut/Image.h"
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Scene;
struct Sphere;
struct Ray;
class Camera;

class Renderer
{
public:
	struct Settings
	{
		bool ShouldAccumulate = true;
	};

public:
	Renderer();

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return _finalImage; }

	void ResetFrameIndex() { _frameIndex = 1; }
	Settings& GetSettings() { return _settings; }

public:
	int Bounces;
	glm::vec3 LightDirection;
	glm::vec3 BackColor;
	bool IsMultiThread = false;
	bool IsMultiThreadInner = false;

private:
	Settings _settings;
	std::shared_ptr<Walnut::Image> _finalImage;

	std::vector<uint32_t> _imageHorIter;
	std::vector<uint32_t> _imageVertIter;

	uint32_t* _imageData = nullptr;
	glm::vec4* _accumulationData = nullptr;

	uint32_t _frameIndex = 1;

	const Scene* _activeScene = nullptr;
	const Camera* _activeCamera = nullptr;

private:
	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		int ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y) const;

	HitPayload TraceRay(const Ray& ray) const;
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex) const;
	HitPayload Miss(const Ray& ray) const;

	bool IntersectSphere(const Ray& ray, const Sphere& sphere, float& closestHit) const;
};
