#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "core/GUILayer.hpp"
#include "graphics/CommandBuffer.hpp"

#include <imgui.h>
#include <optional>
#include <vulkan/vulkan.h>

using namespace Alabaster;

static std::uint32_t quads { 1 };

static glm::vec4 pos { -5, 5, 5, 1.0f };
static glm::vec4 col { 255 / 255.0, 153 / 255.0, 51 / 255.0, 255.0f / 255.0 };
static float ambience { 1.0f };

bool AlabasterLayer::initialise()
{
	command_buffer = CommandBuffer::from_swapchain();
	editor_scene = std::make_unique<SceneSystem::Scene>();

	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	editor_scene->on_event(e);

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

		if (key_code == Key::U) {
			pos.x += 0.1;
			pos.y += 0.1;
		}
		if (key_code == Key::H) {
			pos.x -= 0.1;
			pos.y -= 0.1;
		}

		if (Input::key(Key::G)) {
			Logger::cycle_levels();
			return true;
		}

		return false;
	});
}

void AlabasterLayer::update(float ts) { editor_scene->update(ts); }

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	editor_scene->ui(ts);

	auto viewport_min_region = ImGui::GetWindowContentRegionMin();
	auto viewport_max_region = ImGui::GetWindowContentRegionMax();
	auto viewport_offset = ImGui::GetWindowPos();
	viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
	viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

	viewport_focused = ImGui::IsWindowFocused();
	viewport_hovered = ImGui::IsWindowHovered();

	if (!viewport_hovered)
		Application::the().gui_layer().block_events();
}

void AlabasterLayer::destroy() { }
