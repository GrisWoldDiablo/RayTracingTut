#pragma once


#include "Walnut/Image.h"
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Walnut/Random.h"
#include "Walnut/Random.h"
#include "Walnut/Random.h"
#include "Walnut/Random.h"

class Renderer
{
public:
	Renderer();

	void OnResize(uint32_t width, uint32_t height);
	void Render();
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return _finalImage; }

	float Radius;
	glm::vec3 RayOrigin;
	glm::vec3 LightDir;
	uint32_t SphereColor;
	uint32_t BackColor;
	float _aspectRatio;

private:
	uint32_t PerPixel(glm::vec2 coord);


	std::shared_ptr<Walnut::Image> _finalImage;
	uint32_t* _imageData = nullptr;
};
