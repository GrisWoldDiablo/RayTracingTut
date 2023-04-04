#pragma once


#include "Walnut/Image.h"
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Sphere.h"

struct Ray;
class Camera;

class Renderer
{
public:
	Renderer();

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Camera& camera);
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return _finalImage; }

	Sphere TheSphere;
	Sphere TheSphere2;
	glm::vec3 LightPosition;
	glm::vec4 BackColor;

private:
	glm::vec4 TraceRay(const Ray& ray) const;
	void TraceSphere(const Ray& ray, const Sphere& sphere, float& bestHit, glm::vec4&) const;

	std::shared_ptr<Walnut::Image> _finalImage;
	uint32_t* _imageData = nullptr;
};
