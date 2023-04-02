#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "FileDragDropHandler.hpp"
#include "SceneSystem.hpp"
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

using namespace Alabaster;
using namespace SceneSystem;
using namespace Component;

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
	dispatch.dispatch<MouseButtonPressedEvent>([hovered = viewport_hovered, &scene = editor_scene](const MouseButtonPressedEvent& mouse) {
		const auto is_left_button = mouse.get_mouse_button() == Mouse::Left;
		const auto guizmo_is_over = ImGuizmo::IsOver();
		if (const auto is_hovered = hovered; is_left_button && !guizmo_is_over && is_hovered) {
			scene->update_selected_entity();
		}

		return false;
	});
	dispatch.dispatch<KeyPressedEvent>([this](KeyPressedEvent& key_event) {
		switch (key_event.get_key_code()) {
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
			if (Input::all(Key::LeftShift, Key::LeftControl))
				serialise_scene();
			return false;
		}
		case Key::T: {
			if (gizmo_type == ImGuizmo::OPERATION::TRANSLATE) {
				gizmo_type = ImGuizmo::OPERATION::ROTATE;
				return false;
			}
			if (gizmo_type == ImGuizmo::OPERATION::ROTATE) {
				gizmo_type = ImGuizmo::OPERATION::SCALE;
				return false;
			}
			if (gizmo_type == ImGuizmo::OPERATION::SCALE) {
				gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
				return false;
			}
			return false;
		}
		default: {
			return false;
		}
		}
	});
}

void AlabasterLayer::update(float ts)
{
	editor_scene->update(ts);
	for (const auto& panel : panels) {
		panel->on_update(ts);
	}
}

void AlabasterLayer::render() { editor_scene->render(); }

void AlabasterLayer::ui(float ts)
{
	editor_scene->ui(ts);

	static bool persistent = true;
	const bool opt_fullscreen = persistent;
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
		const float min_window_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
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
	UI::block_events(!viewport_hovered && !viewport_focused);

	ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
	viewport_size = { viewport_panel_size.x, viewport_panel_size.y };
	editor_scene->update_viewport_sizes(viewport_size, viewport_bounds, { viewport_offset.x, viewport_offset.y });

	const auto& img = editor_scene->final_image();
	UI::image(*img, { viewport_size.x, viewport_size.y });

	ui_toolbar();

	if (auto* entity = editor_scene->get_selected_entity(); entity->is_valid()) {
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		const auto& camera = editor_scene->get_camera();
		const auto& camera_view = camera->get_view_matrix();
		const auto& camera_projection = camera->get_projection_matrix();
		auto copy = camera_projection;
		copy[1][1] *= -1.0f;

		auto& entity_transform = entity->get_transform();
		auto transform = entity_transform.to_matrix();

		bool snap = Input::key(Key::LeftShift);
		float snap_value = 0.5f;
		if (gizmo_type == ImGuizmo::OPERATION::ROTATE) {
			snap_value = 45.0f;
		}

		std::array snap_values = { snap_value, snap_value, snap_value };

		ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(copy), gizmo_type, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr,
			snap ? snap_values.data() : nullptr);

		if (ImGuizmo::IsUsing()) {
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(transform, scale, rotation, translation, skew, perspective);

			auto deltaRotation = rotation - entity_transform.rotation;

			entity_transform.position = translation;
			entity_transform.rotation += deltaRotation;
			entity_transform.scale = scale;
		}
	}

	handle_drag_drop();

	ImGui::End();
	ImGui::PopStyleVar();
}

void AlabasterLayer::take_step() { editor_scene->step(); }

void AlabasterLayer::ui_toolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	const auto& colors = ImGui::GetStyle().Colors;
	const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
	const auto& button_active = colors[ImGuiCol_ButtonActive];
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoDecoration;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin("##toolbar", nullptr, window_flags);
	// ImGui::Begin("##toolbar", nullptr);

	const bool toolbar_enabled = static_cast<bool>(editor_scene);

	auto tint_colour = ImVec4(1, 1, 1, 1);
	if (!toolbar_enabled)
		tint_colour.w = 0.5f;

	const float size = ImGui::GetWindowHeight() - 4.0f;
	ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

	const bool has_play_button = scene_state == SceneState::Edit || scene_state == SceneState::Play;
	const bool has_simulate_button = scene_state == SceneState::Edit || scene_state == SceneState::Simulate;
	const bool has_pause_button = scene_state != SceneState::Edit;

	if (has_play_button) {
		if (const auto& icon = (scene_state == SceneState::Edit || scene_state == SceneState::Simulate) ? icon_play : icon_stop;
			UI::image_button(icon, size) && toolbar_enabled) {
			if (scene_state == SceneState::Edit || scene_state == SceneState::Simulate) {
				scene_state = SceneState::Play;
			} else if (scene_state == SceneState::Play) {
				scene_state = SceneState::Edit;
			}
		}
	}

	if (has_simulate_button) {
		if (has_play_button) {
			ImGui::SameLine();
		}

		if (const auto& icon = (scene_state == SceneState::Edit || scene_state == SceneState::Play) ? icon_simulate : icon_stop;
			UI::image_button(icon, size) && toolbar_enabled) {
			if (scene_state == SceneState::Edit || scene_state == SceneState::Play) {
				scene_state = SceneState::Simulate;
			} else if (scene_state == SceneState::Simulate) {
				scene_state = SceneState::Edit;
			}
		}
	}
	if (has_pause_button) {
		const auto is_paused = editor_scene->is_paused();
		ImGui::SameLine();

		if (const auto& icon = icon_pause; UI::image_button(icon, size) && toolbar_enabled) {
			editor_scene->set_paused(!is_paused);
		}

		// Step button
		if (is_paused) {
			ImGui::SameLine();

			if (const auto& icon = icon_step; UI::image_button(icon, size) && toolbar_enabled) {
				take_step();
			}
		}
	}
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
	ImGui::End();
}

void AlabasterLayer::handle_drag_drop() const
{
	const auto path = UI::accept_drag_drop("AlabasterLayer::DragDropPayload");
	if (!path)
		return;

	const std::filesystem::path& fp = *path;
	const auto filename = fp.filename();
	const auto extension = filename.extension();
	auto& scene = *editor_scene;

	try {
		using enum App::Filetype::Filetypes;
		App::handle_filetype<PNG>()(scene, filename);
		App::handle_filetype<TTF>()(scene, filename);
		App::handle_filetype<JPEG>()(scene, filename);
		App::handle_filetype<JPG>()(scene, filename);
		App::handle_filetype<SPV>()(scene, filename);
		App::handle_filetype<VERT>()(scene, filename);
		App::handle_filetype<SCENE>()(scene, filename);
		App::handle_filetype<FRAG>()(scene, filename);
		App::handle_filetype<OBJ>()(scene, filename);
	} catch (const AlabasterException& e) {
		Log::info("[AlabasterLayer] {}", e.what());
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
