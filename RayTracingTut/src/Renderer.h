#pragma once


#include "Walnut/Image.h"
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Sphere.h"

struct Scene;
struct Ray;
class Camera;

class Renderer
{
public:
	Renderer();

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return _finalImage; }

	glm::vec3 LightPosition;
	glm::vec4 BackColor;

private:
	glm::vec4 TraceRay(const Scene& scene, const Ray& ray) const;
	bool IntersectSphere(const Ray& ray, const Sphere& sphere, float& closestHit) const;
	glm::vec4 DrawSphere(const Ray& ray, const Sphere& sphere, float hitPoint) const;

	std::shared_ptr<Walnut::Image> _finalImage;
	uint32_t* _imageData = nullptr;
};

