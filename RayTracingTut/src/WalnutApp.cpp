#include "Walnut/Application.h"
#include "Camera.h"
#include "Renderer.h"
#include "Walnut/EntryPoint.h"
#include "imgui.h"
#include "Scene.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: _camera(45.0f, 0.1f, 100.0f)
	{
		{
			Material& material = _scene.Materials.emplace_back();;
			material.Albedo = {1.0f, 0.4f, 1.0f};
			material.Roughness = 0.0f;

			Sphere sphere;
			sphere.Radius = 1.0f;
			sphere.Position = {0.0f, 0.0f, 0.0f};
			sphere.MaterialIndex = 0;
			_scene.Spheres.push_back(sphere);
		}

		{
			Material& material = _scene.Materials.emplace_back();;
			material.Albedo = {0.2f, 0.9f, 1.0f};
			material.Roughness = 0.02f;

			Sphere sphere;
			sphere.Radius = 100.0f;
			sphere.Position = {0.0f, -101.0f, 0.0f};
			sphere.MaterialIndex = 1;
			_scene.Spheres.push_back(sphere);
		}

		_renderTimes.resize(100);
	}

	virtual void OnUpdate(float ts) override
	{
		if (_camera.OnUpdate(ts))
		{
			_renderer.ResetFrameIndex();
		}
	}

	virtual void OnUIRender() override
	{
		DrawSettings();
		DrawScenes();
		DrawSpheres();
		DrawMaterials();
		DrawViewport();
	}

	void DrawMaterialControl(Material& material) const
	{
		ImGui::ColorEdit3("Color", glm::value_ptr(material.Albedo));
		ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Metallic", &material.Metallic, 0.01f, 0.0f, 1.0f);
	}

	void DrawSphereControl(Sphere& sphere) const
	{
		ImGui::DragFloat("Radius", &sphere.Radius, 0.01f, 1000.0f);
		ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.01f);
		ImGui::DragInt("Material Index", &sphere.MaterialIndex, 1.0f, 0, static_cast<int>(_scene.Materials.size() - 1));
	}

	void DrawSettings()
	{
		ImGui::Begin("Settings");
		if (_lastRenderTime != 0.0f)
		{
			ImGui::Text("Last render: %.3fms", _lastRenderTime);
			ImGui::Text("Min render: %.3fms", _minRenderTime);
			ImGui::Text("Max render: %.3fms", _maxRenderTime);
			ImGui::Text("Average render: %.3fms", _averageRenderTime);
		}
		else
		{
			ImGui::Text("Render for stats.");
		}

		ImGui::Checkbox("MultiThread", &_renderer.IsMultiThread);
		ImGui::Checkbox("MultiThreadInner", &_renderer.IsMultiThreadInner);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("Accumulate", &_renderer.GetSettings().ShouldAccumulate);
		if (ImGui::Button("Reset"))
		{
			_renderer.ResetFrameIndex();
		}

		ImGui::Checkbox("RealTime", &_shouldRender);
		ImGui::DragFloat3("Light Direction", glm::value_ptr(_renderer.LightDirection), 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("BackColor", glm::value_ptr(_renderer.BackColor));
		ImGui::DragInt("Bounces", &_renderer.Bounces, 1, 1, 10);
		ImGui::End();
	}

	void DrawScenes()
	{
		ImGui::Begin("Scene");

		static Sphere newSphere;
		DrawSphereControl(newSphere);

		if (ImGui::Button("Add Sphere"))
		{
			_scene.Spheres.push_back(newSphere);
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear Spheres"))
		{
			_scene.Spheres.clear();
		}

		ImGui::End();
	}

	void DrawSpheres()
	{
		ImGui::Begin("Spheres");

		for (size_t i = 0; i < _scene.Spheres.size(); i++)
		{
			Sphere& sphere = _scene.Spheres[i];
			ImGui::PushID(static_cast<int>(i));
			DrawSphereControl(sphere);
			ImGui::PopID();
			ImGui::Separator();
		}

		ImGui::End();
	}

	void DrawMaterials()
	{
		ImGui::Begin("Materials");

		for (size_t i = 0; i < _scene.Materials.size(); i++)
		{
			Material& material = _scene.Materials[i];
			ImGui::PushID(static_cast<int>(i));
			ImGui::Text(std::format("Index {0}", i).c_str());
			DrawMaterialControl(material);
			ImGui::PopID();
			ImGui::Separator();
		}

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

			if (const auto image = _renderer.GetFinalImage())
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
		_renderer.Render(_scene, _camera);

		_lastRenderTime = timer.ElapsedMillis();
		if (_lastRenderTime < _minRenderTime)
		{
			_minRenderTime = _lastRenderTime;
		}

		if (_lastRenderTime > _maxRenderTime)
		{
			_maxRenderTime = _lastRenderTime;
		}

		_renderTimes[_renderedFrame] = _lastRenderTime;
		_renderedFrame++;
		if (_renderedFrame >= _renderTimes.size())
		{
			_renderedFrame = 0;
		}
		
		float renderTimeTotal = 0;
		for (const float _renderTime : _renderTimes)
		{
			renderTimeTotal += _renderTime;
		}

		_averageRenderTime = renderTimeTotal / static_cast<float>(_renderTimes.size());
	}

private:
	Renderer _renderer;
	Camera _camera;
	Scene _scene;

	uint32_t _viewportWidth = 0;
	uint32_t _viewportHeight = 0;
	float _lastRenderTime = 0.0f;
	float _minRenderTime = FLT_MAX;
	float _maxRenderTime = 0.0f;

	int _renderedFrame = 0;
	std::vector<float> _renderTimes;
	float _averageRenderTime;

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
