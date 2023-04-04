#include "Walnut/Application.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Camera.h"
#include "Renderer.h"
#include "Walnut/EntryPoint.h"
#include "imgui.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: _camera(45.0f, 0.1f, 100.0f) {}

	virtual void OnUpdate(float ts) override
	{
		_camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		DrawSettings();
		DrawViewport();
	}

	void DrawSettings()
	{
		ImGui::Begin("Settings");
		if (_lastRenderTime != 0.0f)
		{
			ImGui::Text("Last render: %.3fms", _lastRenderTime);
			ImGui::Text("Min render: %.3fms", _minRenderTime);
			ImGui::Text("Max render: %.3fms", _maxRenderTime);
		}
		else
		{
			ImGui::Text("Render for stats.");
		}

		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("RealTime", &_shouldRender);

		ImGui::SliderFloat("Radius", &_renderer.TheSphere.Radius, 0.01f, 2.0f);
		ImGui::DragFloat3("Sphere Position", &_renderer.TheSphere.Position.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("Light Position", &_renderer.LightPosition.x, 0.01f, -10.0f, 10.0f);
		ImGui::ColorPicker4("SphereColor", &_renderer.TheSphere.Albedo.r);
		ImGui::ColorPicker4("BackColor", &_renderer.BackColor.r);
		ImGui::End();
	}

	void DrawViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		_viewportWidth = static_cast<uint32_t>(ImGui::GetContentRegionAvail().x > 0 ? ImGui::GetContentRegionAvail().x : 0);
		_viewportHeight = static_cast<uint32_t>(ImGui::GetContentRegionAvail().y > 0 ? ImGui::GetContentRegionAvail().y : 0);
		if (_viewportWidth > 0 && _viewportHeight > 0)
		{
			if (_shouldRender)
			{
				Render();
			}

			if (auto image = _renderer.GetFinalImage())
			{
				ImGui::Image(image->GetDescriptorSet(),
					{static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight())},
					ImVec2(0, 1), ImVec2(1, 0));
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Render()
	{
		Timer timer;
		// Renderer resize
		_renderer.OnResize(_viewportWidth, _viewportHeight);
		_camera.OnResize(_viewportWidth, _viewportHeight);
		// Renderer render
		_renderer.Render(_camera);

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
	Renderer _renderer;
	Camera _camera;
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

	auto* app = new Walnut::Application(spec);
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
