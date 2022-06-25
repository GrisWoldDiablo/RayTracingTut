#pragma once


#include "Walnut/Image.h"
#include <memory>
#include <glm/vec2.hpp>

class Renderer
{
public:
	Renderer();

	void OnResize(uint32_t width, uint32_t height);
	void Render();
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return _finalImage; }

	float Radius;

private:
	uint32_t PerPixel(glm::vec2 coord);


	std::shared_ptr<Walnut::Image> _finalImage;
	uint32_t* _imageData = nullptr;
};
