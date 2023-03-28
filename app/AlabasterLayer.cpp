#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "SceneSystem.hpp"
#include "core/Common.hpp"
#include "core/GUILayer.hpp"
#include "core/Logger.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "graphics/CommandBuffer.hpp"
#include "panels/DirectoryContentPanel.hpp"
#include "panels/SceneEntitiesPanel.hpp"
#include "panels/StatisticsPanel.hpp"
#include "ui/ImGuizmo.hpp"

#include <glm/ext/matrix_relational.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui.h>
#include <optional>
#include <string.h>
#include <vulkan/vulkan.h>

using namespace Alabaster;
using namespace SceneSystem;
using namespace Component;

static std::uint32_t quads { 1 };
static bool is_dockspace_open { true };

static bool global_imgui_is_blocking { false };

bool AlabasterLayer::initialise()
{
	editor_scene = std::make_unique<Scene>();
	editor_scene->initialise();

	panels.push_back(std::make_unique<App::SceneEntitiesPanel>(*editor_scene));
	panels.push_back(std::make_unique<App::DirectoryContentPanel>(IO::resources()));
	panels.push_back(std::make_unique<App::StatisticsPanel>(Application::the().get_statistics()));

	auto& watcher = Application::the().get_file_watcher();
	for (const auto& panel : panels) {
		panel->on_init();
		panel->register_file_watcher(watcher);
	}
	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	for (const auto& panel : panels) {
		panel->on_event(e);
	}

	editor_scene->on_event(e);

	EventDispatcher dispatch(e);
	dispatch.dispatch<MouseScrolledEvent>([](MouseScrolledEvent&) { return global_imgui_is_blocking; });
	dispatch.dispatch<MouseButtonPressedEvent>([hovered = viewport_hovered, &scene = editor_scene](MouseButtonPressedEvent& mouse) {
		const auto is_left_button = mouse.get_mouse_button() == Mouse::Left;
		const auto guizmo_is_over = ImGuizmo::IsOver();
		const auto is_hovered = hovered;
		if (is_left_button && !guizmo_is_over && is_hovered) {
			scene->update_selected_entity();
		}

		return false;
	});
	dispatch.dispatch<KeyPressedEvent>([this](KeyPressedEvent& key_event) {
		switch (const auto key_code = key_event.get_key_code()) {
		case Key::Escape: {
			Application::the().exit();
			return true;
		}
		case Key::F: {
			UI::empty_cache();
			return false;
		}
		case Key::G: {
			Logger::cycle_levels();
			return false;
		}
		case Key::S: {
			serialise_scene();
			return false;
		}
		default:
			return false;
		}

		return false;
	});
}

void AlabasterLayer::update(float ts)
{
	editor_scene->update(ts);
	for (const auto& panel : panels) {
		panel->on_update(ts);
	}
}

void AlabasterLayer::ui(float ts)
{
	editor_scene->ui(ts);

	static bool persistent = true;
	bool opt_fullscreen = persistent;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen) {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
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
		const ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float min_window_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_window_size_x;

		menu_bar();

		for (const auto& panel : panels) {
			panel->ui(ts);
		}

		viewport();
	}
	ImGui::End();
}

void AlabasterLayer::menu_bar() const
{
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
}

void AlabasterLayer::viewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar);
	auto viewport_min_region = ImGui::GetWindowContentRegionMin();
	auto viewport_max_region = ImGui::GetWindowContentRegionMax();
	auto viewport_offset = ImGui::GetWindowPos();
	viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
	viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

	viewport_focused = ImGui::IsWindowFocused();
	viewport_hovered = ImGui::IsWindowHovered();

	global_imgui_is_blocking = !viewport_hovered && !viewport_focused;
	Application::the().gui_layer().block_events(!viewport_hovered && !viewport_focused);
	ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
	viewport_size = { viewport_panel_size.x, viewport_panel_size.y };
	editor_scene->update_viewport_sizes(viewport_size, viewport_bounds, { viewport_offset.x, viewport_offset.y });

	const auto& img = editor_scene->final_image();
	UI::image(*img, { viewport_size.x, viewport_size.y });

	if (auto* entity = editor_scene->get_selected_entity(); entity->is_valid()) {
		ImGuizmo::SetDrawlist();

		ImGuizmo::SetRect(
			viewport_bounds[0].x, viewport_bounds[0].y, viewport_bounds[1].x - viewport_bounds[0].x, viewport_bounds[1].y - viewport_bounds[0].y);

		const auto& camera = editor_scene->get_camera();
		const auto& camera_view = camera->get_view_matrix();
		const auto& camera_projection = camera->get_projection_matrix();

		auto& entity_transform = entity->get_transform();
		auto transform = entity_transform.to_matrix();

		ImGuizmo::Manipulate(
			glm::value_ptr(camera_view), glm::value_ptr(camera_projection), ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(transform));

		if (ImGuizmo::IsUsing()) {
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(transform, scale, rotation, translation, skew, perspective);

			entity_transform.position = translation;
			entity_transform.rotation = rotation;
			entity_transform.scale = scale;
		}
	}

	handle_drag_drop();

	ImGui::End();
	ImGui::PopStyleVar();
}

template <> struct handle_filetype<Filetype::Filetypes::PNG> {
	void operator()(SceneSystem::Scene& scene, const std::filesystem::path& path) const
	{
		if (path.extension() != ".png")
			return;

		TextureProperties props;
		const auto img = Alabaster::Texture::from_filename(path, props);
		auto entity = scene.create_entity("TestEntity");
		entity.add_component<Component::Texture>(glm::vec4 { 1.0 }, img);
		return;
	}
};

template <> struct handle_filetype<Filetype::Filetypes::SCENE> {
	void operator()(SceneSystem::Scene& scene, const std::filesystem::path& path) const
	{
		if (path.extension() != ".scene")
			return;

		SceneSystem::SceneDeserialiser deserialiser(path, scene);
		deserialiser.deserialise();
	}
};

template <> struct handle_filetype<Filetype::Filetypes::OBJ> {
	void operator()(SceneSystem::Scene& scene, const std::filesystem::path& path) const
	{
		if (path.extension() != ".obj")
			return;

		const auto mesh = Alabaster::Mesh::from_file(path);
		auto entity = scene.create_entity("TestEntity");
		entity.add_component<Component::Mesh>(mesh);
		entity.add_component<Component::Pipeline>();
		return;
	}
};

void AlabasterLayer::handle_drag_drop()
{
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AlabasterLayer::DragDropPayload")) {
#ifndef ALABASTER_WINDOWS
			const auto* path = static_cast<const char*>(payload->Data);
#else
			const auto* path = static_cast<const wchar_t*>(payload->Data);
#endif
			const std::filesystem::path fp = path;
			const auto filename = fp.filename();
			const auto extension = filename.extension();
			auto& scene = *editor_scene;

			try {
				using enum Filetype::Filetypes;
				handle_filetype<PNG>()(scene, filename);
				handle_filetype<TTF>()(scene, filename);
				handle_filetype<JPEG>()(scene, filename);
				handle_filetype<JPG>()(scene, filename);
				handle_filetype<SPV>()(scene, filename);
				handle_filetype<VERT>()(scene, filename);
				handle_filetype<SCENE>()(scene, filename);
				handle_filetype<FRAG>()(scene, filename);
				handle_filetype<OBJ>()(scene, filename);
			} catch (const AlabasterException& e) {
				Log::info("[AlabasterLayer] {}", e.what());
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void AlabasterLayer::destroy()
{
	editor_scene->shutdown();
	for (auto const& panel : panels) {
		panel->on_destroy();
	}
}

void AlabasterLayer::serialise_scene() { SceneSerialiser serialiser(*editor_scene); }
