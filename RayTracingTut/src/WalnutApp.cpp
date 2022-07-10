#include "Walnut/Application.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Renderer.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
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

		ImGui::SliderFloat("Radius", &_renderer.Radius, 0.01f, 2.0f);
		ImGui::DragFloat3("Light", &_renderer.LightDir.x, 0.01f, -2.0f, 2.0f);
		//const ImU32   u32_zero = 0, u32_one = 1, u32_fifty = 50, u32_min = 0, u32_max = UINT_MAX / 2, u32_hi_a = UINT_MAX / 2 - 100, u32_hi_b = UINT_MAX / 2;
		auto valueF = ImGui::ColorConvertU32ToFloat4(_renderer.SphereColor);
		ImGui::ColorPicker4("SphereColor", &valueF.x);
		auto valueU = ImGui::ColorConvertFloat4ToU32(valueF);
		_renderer.SphereColor = valueU;
		valueF = ImGui::ColorConvertU32ToFloat4(_renderer.BackColor);
		ImGui::ColorPicker4("BackColor", &valueF.x);
		valueU = ImGui::ColorConvertFloat4ToU32(valueF);
		_renderer.BackColor = valueU;


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
					{ static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight()) },
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
		// Renderer render
		_renderer.Render();

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