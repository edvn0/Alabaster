#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "graphics/Renderer.hpp"
#include "vulkan/vulkan_core.h"

#include <imgui.h>

using namespace Alabaster;

static const std::vector<Vertex> vertices { Vertex { .position = { -0.5, 0.5, 0, 1 }, .colour = { 1, 0, 0, 1 }, .uv = { -1, 1 } },
	Vertex { .position = { 0.5, 0.5, 0, 1 }, .colour = { 1, 0, 0, 1 }, .uv = { 1, 1 } },
	Vertex { .position = { 0.5, -0.5, 0, 1 }, .colour = { 1, 1, 0, 1 }, .uv = { 1, -1 } },
	Vertex { .position = { -0.5, -0.5, 0, 1 }, .colour = { 1, 0, 1, 1 }, .uv = { -1, -1 } } };

static const std::vector<Index> indices { 0, 1, 2, 2, 3, 0 };

static uint32_t quads { 1 };

bool AlabasterLayer::initialise()
{
	vertex_buffer = VertexBuffer::create(vertices);
	index_buffer = IndexBuffer::create(indices);

	aeroplane_texture = std::make_unique<Texture2D>("app/resources/textures/aeroplane.png");
	uint32_t black = 0x00000000;
	black_texture = std::make_unique<Texture2D>(&black, sizeof(uint32_t));

	viking_room_model = Mesh::from_path("app/resources/models/viking_room.obj");
	square_model = Mesh::from_data(vertices, indices);
	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	camera.on_event(e);

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
	camera.on_update(ts);

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			renderer.quad(glm::vec4 { x, y, 0, 0 }, glm::vec4 { x / 5.0f, 1, y / 5.0f, 1 }, glm::vec3 { 0.2, 0.2, 0.2 });
		}
	}

	renderer.line({ 0, 0, 0 }, { 3, 0, 0 }, { 1, 0, 0, 1 });
	renderer.line({ 0, 0, 0 }, { 0, 3, 0 }, { 0, 1, 0, 1 });
	renderer.line({ 0, 0, 0 }, { 0, 0, 3 }, { 0, 0, 1, 1 });

	// renderer.mesh(viking_room_model, viking_room_pipeline);
	renderer.end_scene();

	frame_number++;
}

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	static bool persistent = true;
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
			ImGui::Text("Frametime: %s", std::to_string(frametime).c_str());
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
	vkDestroyRenderPass(GraphicsContext::the().device(), render_pass, nullptr);
	viking_room_model->destroy();
	viking_room_pipeline->destroy();
	square_model->destroy();

	vertex_buffer->destroy();
	index_buffer->destroy();
	graphics_pipeline->destroy();

	Log::info("[AlabasterLayer] Destroyed layer.");
	Layer::destroy();
}
