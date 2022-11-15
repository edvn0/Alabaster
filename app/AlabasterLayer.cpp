#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AlabasterShaderCompiler.hpp"
#include "graphics/Renderer.hpp"

#include <imgui.h>
#include <vulkan/vulkan.h>

using namespace Alabaster;

static uint32_t quads { 1 };

bool AlabasterLayer::initialise()
{
	viking_room_model = Mesh::from_file("viking_room.obj");
	sphere_model = Mesh::from_file("sphere.obj");

	sponza_model = Mesh::from_file("sponza.obj");

	for (size_t i = 0; i < 3; i++) {
		PipelineSpecification mesh_spec {
			.shader = Shader("mesh"),
			.debug_name = "Mesh Pipeline",
			.render_pass = renderer.get_render_pass(),
			.wireframe = false,
			.backface_culling = false,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.depth_test = true,
			.depth_write = true,
			.vertex_layout = VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"),
				VertexBufferElement(ShaderDataType::Float4, "colour"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4)),
				PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec4)) },
		};
		test_pipelines[i] = Pipeline::create(mesh_spec);
	}

	// auto shader = AlabasterShaderCompiler::ShaderCache::the().get_from_cache("mesh");

	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	editor.on_event(e);

	EventDispatcher dispatch(e);
	dispatch.dispatch<KeyPressedEvent>([](KeyPressedEvent& key_event) {
		const auto key_code = key_event.get_key_code();
		if (key_code == Key::Escape) {
			Application::the().exit();
			return true;
		}

		if (key_code == Key::C) {
			quads++;
			return true;
		}

		if (Input::key(Key::G)) {
			Logger::cycle_levels();
			return true;
		}

		return false;
	});
}

void AlabasterLayer::update(float ts)
{
	static size_t frame_number { 0 };

	renderer.reset_stats();
	renderer.begin_scene();
	{
		editor.on_update(ts);

		/*for (int x = -10; x <= 10; x++) {
			for (int y = -10; y <= 10; y++) {
				auto xf = static_cast<float>(x) / 10;
				auto yf = static_cast<float>(y) / 10;

					  renderer.quad(glm::vec4 { xf, yf, sin(xf + yf) * cos(xf + yf), 0 }, glm::vec4 { 0.1, 0.9, 0.1, 1.0f },
						  glm::vec3 { 0.2, 0.2, 0.2 }, frame_number % 360);
			}
		}*/

		renderer.quad({ 0, 0, 0 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);

		glm::vec3 axis_base { 0, -0.1, 0 };
		renderer.line(axis_base, axis_base + glm::vec3 { 1, 0, 0 }, { 1, 0, 0, 1 });
		renderer.line(axis_base, axis_base + glm::vec3 { 0, -1, 0 }, { 0, 1, 0, 1 });
		renderer.line(axis_base, axis_base + glm::vec3 { 0, 0, -1 }, { 0, 0, 1, 1 });

		/*auto sphere_rot = glm::mat4 { 1.0f };

		renderer.mesh(sphere_model, nullptr, { 0, 0, 0 }, std::move(sphere_rot), { 1, 0, 1, 1 }, { .1, .1, .1 });

		auto rotation = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
		renderer.mesh(viking_room_model, test_pipelines[0], { 0, 0, 0 }, std::move(rotation), { 1, 1, 1, 1 }, { 1, 1, 1 });

		auto rotation2 = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
		renderer.mesh(viking_room_model, test_pipelines[1], { 3, 3, 0 }, std::move(rotation2), { 1, 1, 1, 1 }, { 1, 1, 1 });
		auto rotation3 = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
		renderer.mesh(viking_room_model, test_pipelines[2], { -3, -3, 0 }, std::move(rotation3), { 1, 1, 1, 1 }, { 1, 1, 1 });
	*/

		renderer.mesh(sponza_model, nullptr, { 0, 0, 0 }, glm::mat4 { 1.0f }, { 1, 0, 0, 1 }, { 0.01, 0.01, 0.01 });
	}
	renderer.end_scene();

	frame_number++;
}

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	static bool persistent = false;
	bool opt_fullscreen = persistent;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen) {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", &is_dockspace_open, window_flags);
	ImGui::PopStyleVar();
	{
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float min_window_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_window_size_x;

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl+N")) { }
				if (ImGui::MenuItem("Open...", "Ctrl+O")) { }
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) { }
				if (ImGui::MenuItem("Exit")) {
					Application::the().exit();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Stats");
		{
			ImGui::Text("Renderer Stats:");
			std::string name = "None";
			ImGui::Text("Hovered Entity: %s", name.c_str());
			auto frametime = Application::the().frametime();
			ImGui::Text("Frame time: %s", std::to_string(frametime).c_str());
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
		{
			ImGui::Begin("Viewport");
			{
				auto viewport_min_region = ImGui::GetWindowContentRegionMin();
				auto viewport_max_region = ImGui::GetWindowContentRegionMax();
				auto viewport_offset = ImGui::GetWindowPos();
				viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
				viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

				viewport_focused = ImGui::IsWindowFocused();
				viewport_hovered = ImGui::IsWindowHovered();

				ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
				viewport_size = { viewport_panel_size.x, viewport_panel_size.y };

				ImVec2 vp_size = ImVec2 { viewport_size.x, viewport_size.y };

				ImGui::End();
			}
		}
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

void AlabasterLayer::destroy()
{
	renderer.destroy();
	viking_room_model->destroy();
	sphere_model->destroy();
	sponza_model->destroy();

	for (auto& pipe : test_pipelines) {
		pipe->destroy();
	}
}
