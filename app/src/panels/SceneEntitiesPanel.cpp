#include "panels/SceneEntitiesPanel.hpp"

#include "component/Component.hpp"
#include "ui/ImGui.hpp"

#include <AssetManager.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace App {

	static constexpr float font_size = 11.0f;
	static constexpr float frame_padding = 0.5f;

	static void draw_vec3_control(const std::string& label, glm::vec3& values, float reset_value = 0.0f, float column_width = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto bold_font = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, column_width);
		ImGui::Text("%s", label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 button_size = { line_height + 3.0f, line_height };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(bold_font);
		if (ImGui::Button("X", button_size))
			values.x = reset_value;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(bold_font);
		if (ImGui::Button("Y", button_size))
			values.y = reset_value;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(bold_font);
		if (ImGui::Button("Z", button_size))
			values.z = reset_value;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template <class Vec>
	static void draw_four_component_vector(const std::string& label, Vec& values, float reset_value = 0.0f, float column_width = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto bold_font = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		{
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, column_width);
			ImGui::Text("%s", label.c_str());
			ImGui::NextColumn();
		}

		{
			ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });
		}

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 button_size = { line_height + 3.0f, line_height };

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("X", button_size))
				values.x = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("Y", button_size))
				values.y = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("Z", button_size))
				values.z = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("W", button_size))
				values.w = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##W", &values.w, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
		}

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template <SceneSystem::Component::IsComponent T>
	static void draw_component(SceneSystem::Entity& entity, const std::string& name, auto&& ui_function)
	{
		if (!entity.has_component<T>())
			return;

		const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		auto& component = entity.get_component<T>();
		ImVec2 content_region_available = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 4, 4 });
		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y + frame_padding * 2.0f;
		ImGui::Separator();

		const auto leaf_id = (void*)typeid(T).hash_code();
		bool open = ImGui::TreeNodeEx(leaf_id, tree_node_flags, "%s", name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(content_region_available.x - line_height * 0.5f);
		if (ImGui::Button("+", ImVec2 { line_height, line_height })) {
			ImGui::OpenPopup("Component Settings");
		}

		bool remove_component = false;
		if (ImGui::BeginPopup("Component Settings")) {
			if (ImGui::MenuItem("Remove component"))
				remove_component = true;

			ImGui::EndPopup();
		}

		if (open) {
			ui_function(component);
			ImGui::TreePop();
		}

		if (remove_component)
			entity.remove_component<T>();
	}

	void SceneEntitiesPanel::draw_components(SceneSystem::Entity& entity)
	{
		if (entity.has_component<SceneSystem::Component::Tag>()) {
			auto& tag = entity.get_tag().tag;

			std::string buffer;
			buffer.reserve(500);
			std::memset(buffer.data(), 0, sizeof(buffer));
			tag.copy(buffer.data(), 499);
			if (ImGui::InputText("##Tag", buffer.data(), sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent")) {
			display_add_component_entry<SceneSystem::Component::Camera>("Camera");
			display_add_component_entry<SceneSystem::Component::Texture>("Texture");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		draw_component<SceneSystem::Component::Transform>(entity, "Transform", [](SceneSystem::Component::Transform& component) {
			draw_vec3_control("Translation", component.position);
			draw_four_component_vector("Rotation", component.rotation);
			draw_vec3_control("Scale", component.scale, 1.0f);
		});

		draw_component<SceneSystem::Component::Texture>(entity, "Texture", [](SceneSystem::Component::Texture& component) {
			ImGui::ColorEdit4("Colour", glm::value_ptr(component.colour));

			if (component.texture)
				Alabaster::UI::image(component.texture->get_descriptor_info(), ImVec2(200, 200));
		});

		draw_component<SceneSystem::Component::SphereIntersectible>(entity, "SphereIntersectible",
			[](SceneSystem::Component::SphereIntersectible& component) { draw_vec3_control("World position", component.world_position); });

		draw_component<SceneSystem::Component::Camera>(entity, "Camera", [](SceneSystem::Component::Camera& component) {
			const auto name = Alabaster::enum_name(component.camera_type);
			ImGui::Text("Type: %s", name.data());
		});

		draw_component<SceneSystem::Component::Mesh>(entity, "Mesh",
			[](const SceneSystem::Component::Mesh& component) { ImGui::Text("Mesh path: %s", component.mesh->get_asset_path().string().data()); });

		draw_component<SceneSystem::Component::Pipeline>(entity, "Pipeline", [](SceneSystem::Component::Pipeline& component) {
			auto& info = component.pipeline->get_specification();
			ImGui::Text("Pipeline %s\nLine width: %f", info.debug_name.c_str(), info.line_width);
			ImGui::Button("Pipeline shader");
			const auto path = Alabaster::UI::accept_drag_drop("AlabasterLayer::DragDropPayload");
			if (!path)
				return;

			auto fp = *path;
			const auto filename_wo_extension = fp.filename().replace_extension();
			const auto directory = fp.parent_path();
			const auto extension = fp.extension();

			const auto vertex_filename = (std::string { filename_wo_extension.string() } + ".vert");
			const auto fragment_filename = (std::string { filename_wo_extension.string() } + ".frag");

			const auto vertex = directory / vertex_filename;
			const auto fragment = directory / fragment_filename;

			const auto vertex_is_file = Alabaster::IO::is_file(vertex);
			const auto fragment_is_file = Alabaster::IO::is_file(fragment);
			if (!(vertex_is_file && fragment_is_file)) {
				Alabaster::Log::info("Vertex or fragment shader missing.");
				return;
			}

			AssetManager::ShaderCompiler compiler;
			auto shader = compiler.compile(fp.string(), Alabaster::IO::shader(vertex_filename), Alabaster::IO::shader(fragment_filename));
			(void)shader.descriptor_set_layouts();

			info.shader = std::make_shared<Alabaster::Shader>(shader);
			try {
				component.pipeline->invalidate();
			} catch (const std::exception& e) {
				Alabaster::Log::info("{}", e.what());
			}
		});
	}

	void SceneEntitiesPanel::draw_entity_node(SceneSystem::Entity& entity)
	{
		const auto& tag = entity.get_tag().tag;

		ImGuiTreeNodeFlags flags = ((selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		const auto& id = entity.get_component<SceneSystem::Component::ID>();
		bool opened = ImGui::TreeNodeEx(id.identifier.as_bytes().data(), flags, "%s", tag.c_str());
		if (ImGui::IsItemClicked()) {
			selected_entity = entity;
		}

		bool entity_deleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity"))
				entity_deleted = true;

			ImGui::EndPopup();
		}

		if (opened) {
			ImGuiTreeNodeFlags opened_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			const auto leaf_id = (const char*)&entity.get_component<SceneSystem::Component::ID>().identifier;
			if (bool was_opened = ImGui::TreeNodeEx(leaf_id, opened_flags, "%s", tag.c_str()))
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entity_deleted) {
			scene.delete_entity(entity);
			if (selected_entity == entity)
				selected_entity = {};
		}
	}

	void SceneEntitiesPanel::on_update(float)
	{
		if (const auto* picked_entity = scene.get_selected_entity(); picked_entity && picked_entity->is_valid()) {
			selected_entity = *picked_entity;
		}
	}

	void SceneEntitiesPanel::ui(float ts)
	{
		ImGui::Begin("Scene Hierarchy");

		scene.for_each_entity([this](auto entity_id) {
			auto entity = scene.create_entity(entity_id);
			draw_entity_node(entity);
		});

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow("EmptyEntityId", 1)) {
			if (ImGui::MenuItem("Create Empty Entity"))
				scene.create_entity("Empty Entity");

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (selected_entity.is_valid()) {
			draw_components(selected_entity);
		}

		ImGui::End();
	}

	void SceneEntitiesPanel::on_event(Alabaster::Event&) { }

} // namespace App
