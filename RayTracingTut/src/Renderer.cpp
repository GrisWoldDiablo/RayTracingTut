#include "Renderer.h"

#include "Walnut/Random.h"

Renderer::Renderer()
	:Radius(0.5f)
{}

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

void Renderer::Render()
{
	// render every pixel
	for (uint32_t y = 0; y < _finalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < _finalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { static_cast<float>(x) / static_cast<float>(_finalImage->GetWidth()), static_cast<float>(y) / static_cast<float>(_finalImage->GetHeight()) };
			coord = coord * 2.0f - 1.0f;
			_imageData[x + y * _finalImage->GetWidth()] = PerPixel(coord);
		}

	}

	_finalImage->SetData(_imageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	auto r = static_cast<uint8_t>(coord.x * 255.0f);
	auto g = static_cast<uint8_t>(coord.y * 255.0f);

	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	//rayDirection = glm::normalize(rayDirection);
	//float radius = 0.7f;
	//Radius = 0.7f;

	// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0

	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	//float a = rayDirection.x * rayDirection.x
	//		+ rayDirection.y * rayDirection.y
	//		+ rayDirection.z * rayDirection.z;

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - Radius * Radius;

	// Quadratic formula discriminant
	// b^2 - 4ac

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant >= 0.0f)
	{
		return 0xff000000 | (g << 8) | r;
	}

	return 0xff000000;

	return 0xff000000 | (g << 8) | r;
}
