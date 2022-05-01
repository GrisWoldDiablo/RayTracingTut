#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		DrawSettings();
		DrawViewport();
	}

	void DrawSettings()
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", _lastRenderTime);
		ImGui::Text("Min render: %.3fms", _minRenderTime);
		ImGui::Text("Max render: %.3fms", _maxRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}
		if (ImGui::Button("Toggle RealTime"))
		{
			_shouldRender = !_shouldRender;
		}
		ImGui::End();
	}

	void DrawViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		_viewportWidth = ImGui::GetContentRegionAvail().x;
		_viewportHeight = ImGui::GetContentRegionAvail().y;

		if (_shouldRender)
		{
			Render();
		}

		if (_image)
		{
			ImGui::Image(_image->GetDescriptorSet(), { (float)_image->GetWidth(),(float)_image->GetHeight() });
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Render()
	{
		Timer timer;
		uint32_t size = _viewportWidth * _viewportHeight;
		if (!_image || _viewportWidth != _image->GetWidth() || _viewportHeight != _image->GetHeight())
		{
			_image = std::make_shared<Image>(_viewportWidth, _viewportHeight, ImageFormat::RGBA);
			delete[] _imageData;
			_imageData = new uint32_t[size];
		}

		for (uint32_t i = 0; i < size; i++)
		{
			_imageData[i] = Random::UInt();
			_imageData[i] |= 0xff000000;
		}
		_image->SetData(_imageData);

		_lastRenderTime = timer.ElapsedMillis();
		if (_lastRenderTime < _minRenderTime)
		{
			_minRenderTime = _lastRenderTime;
		}

		if (_lastRenderTime > _maxRenderTime)
		{
			_maxRenderTime = _lastRenderTime;
		}
	}

private:
	std::shared_ptr<Image> _image = nullptr;
	uint32_t* _imageData = nullptr;
	uint32_t _viewportWidth = 0;
	uint32_t _viewportHeight = 0;
	float _lastRenderTime = 0.0f;
	float _minRenderTime = FLT_MAX;
	float _maxRenderTime = 0.0f;
	bool _shouldRender = false;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}