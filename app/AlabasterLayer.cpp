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

void AlabasterLayer::create_renderpass()
{
	VkAttachmentDescription color_attachment {};
	color_attachment.format = Application::the().get_window()->get_swapchain()->get_format();
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_ref {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &render_pass_info, nullptr, &render_pass));
}

bool AlabasterLayer::initialise()
{
	create_renderpass();

	PipelineSpecification spec {
		.shader = Shader("app/resources/shaders/main"),
		.debug_name = "Test",
		.render_pass = render_pass,
		.wireframe = false,
		.backface_culling = false,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.depth_test = true,
		.depth_write = false,
		.vertex_layout = VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"),
			VertexBufferElement(ShaderDataType::Float4, "colour"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
		.instance_layout = {},
	};
	graphics_pipeline = std::make_unique<Pipeline>(spec);
	graphics_pipeline->invalidate();

	vertex_buffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
	index_buffer = std::make_unique<IndexBuffer>(indices.data(), indices.size());

	aeroplane_texture = std::make_unique<Texture2D>("app/resources/textures/aeroplane.png");
	uint32_t black = 0x00000000;
	black_texture = std::make_unique<Texture2D>(&black, sizeof(uint32_t));

	car_model = Mesh::from_path("app/resources/models/car_model.obj");
	square_model = Mesh::from_data(vertices, indices);
	return true;
}

void AlabasterLayer::on_event(Event& e)
{
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

		return false;
	});
}

void AlabasterLayer::update(float ts)
{
	static size_t frame_number { 0 };

    renderer.reset_stats();
	renderer.begin_scene();
    camera.on_update(ts);
	for (uint32_t i = 0; i < quads; i++) {
        renderer.quad();
	}
    // renderer.mesh(car_model, graphics_pipeline);
	renderer.end_scene();

	handle_events();

	frame_number++;
}

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	static bool persistent = true;
	bool opt_fullscreen = persistent;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
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

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the
	// pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
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
	car_model->destroy();
	square_model->destroy();

	vertex_buffer->destroy();
	index_buffer->destroy();
	graphics_pipeline->destroy();

	Log::info("[AlabasterLayer] Destroyed layer.");
	Layer::destroy();
}

void AlabasterLayer::handle_events()
{
	if (Input::key(Key::G)) {
		Logger::cycle_levels();
		return;
	}
}
