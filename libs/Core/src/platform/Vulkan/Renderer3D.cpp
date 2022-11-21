#include "av_pch.hpp"

#include "graphics/Renderer3D.hpp"

#include "AssetManager.hpp"
#include "core/Application.hpp"
#include "core/Window.hpp"
#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantRange.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Shader.hpp"
#include "graphics/UniformBuffer.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBuffer.hpp"
#include "utilities/FileInputOutput.hpp"
#include "vulkan/vulkan_core.h"

#include <vulkan/vulkan.h>

#define ALABASTER_USE_IMGUI

namespace Alabaster {

	struct RendererTransform {
		alignas(16) glm::mat4 transform;
		alignas(16) glm::vec4 colour;
	};

	static constexpr auto default_model = glm::mat4 { 1.0f };

	static void reset_data(RendererData& to_reset)
	{
		to_reset.quad_indices_submitted = 0;
		to_reset.quad_vertices_submitted = 0;
		to_reset.line_indices_submitted = 0;
		to_reset.line_vertices_submitted = 0;
		to_reset.meshes_submitted = 0;
	}

	void Renderer3D::create_renderpass()
	{
		const auto&& [color, depth] = Application::the().swapchain().get_formats();

		VkAttachmentDescription color_attachment_desc = {};
		color_attachment_desc.format = color;
		color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
#ifdef ALABASTER_USE_IMGUI
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#else
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
#endif
		color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkAttachmentDescription depth_attachment_desc {};
		depth_attachment_desc.format = depth;
		depth_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_reference = {};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_reference = {};
		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_reference;
		subpass_description.pDepthStencilAttachment = &depth_reference;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> descriptions { color_attachment_desc, depth_attachment_desc };
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.pAttachments = descriptions.data();
		render_pass_info.attachmentCount = static_cast<std::uint32_t>(descriptions.size());
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.subpassCount = 1;
		render_pass_info.pDependencies = &dependency;
		render_pass_info.dependencyCount = 1;

		vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &render_pass_info, nullptr, &data.render_pass));
	}

	void Renderer3D::create_descriptor_set_layout()
	{
		VkDescriptorSetLayoutBinding ubo_layout_binding {};
		ubo_layout_binding.binding = 0;
		ubo_layout_binding.descriptorCount = 1;
		ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.pImmutableSamplers = nullptr;
		ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;

		VkDescriptorSetLayoutBinding combined_image_sampler {};
		combined_image_sampler.binding = 1;
		combined_image_sampler.descriptorCount = 1;
		combined_image_sampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		combined_image_sampler.pImmutableSamplers = nullptr;
		combined_image_sampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { ubo_layout_binding, combined_image_sampler };
		VkDescriptorSetLayoutCreateInfo layout_info {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		vk_check(vkCreateDescriptorSetLayout(GraphicsContext::the().device(), &layout_info, nullptr, &data.descriptor_set_layout));
	}

	void Renderer3D::create_descriptor_pool()
	{
		std::array<VkDescriptorPoolSize, 2> pool_sizes {};
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = static_cast<std::uint32_t>(Application::the().swapchain().get_image_count());

		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[1].descriptorCount = static_cast<std::uint32_t>(Application::the().swapchain().get_image_count());

		VkDescriptorPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<std::uint32_t>(Application::the().swapchain().get_image_count());

		if (vkCreateDescriptorPool(GraphicsContext::the().device(), &pool_info, nullptr, &data.descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void Renderer3D::create_descriptor_sets()
	{
		const auto image_count = Application::the().swapchain().get_image_count();

		std::vector<VkDescriptorSetLayout> layouts(image_count, data.descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = data.descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<std::uint32_t>(image_count);
		alloc_info.pSetLayouts = layouts.data();

		data.descriptor_sets.resize(image_count);
		vk_check(vkAllocateDescriptorSets(GraphicsContext::the().device(), &alloc_info, data.descriptor_sets.data()));

		const auto image_info = data.viking_room_texture->vulkan_image_info();
		for (std::size_t i = 0; i < image_count; i++) {
			VkDescriptorBufferInfo buffer_info {};
			buffer_info.buffer = data.uniforms[i].get_buffer();
			buffer_info.offset = 0;
			buffer_info.range = sizeof(UBO);

			std::array<VkWriteDescriptorSet, 2> descriptor_writes {};
			auto& ubo = descriptor_writes[0];
			auto& combined_sampler = descriptor_writes[1];

			ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			ubo.dstSet = data.descriptor_sets[i];
			ubo.dstBinding = 0;
			ubo.dstArrayElement = 0;
			ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			ubo.descriptorCount = 1;
			ubo.pBufferInfo = &buffer_info;

			combined_sampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			combined_sampler.dstSet = data.descriptor_sets[i];
			combined_sampler.dstBinding = 1;
			combined_sampler.dstArrayElement = 0;
			combined_sampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			combined_sampler.descriptorCount = 1;
			combined_sampler.pImageInfo = &image_info;

			vkUpdateDescriptorSets(
				GraphicsContext::the().device(), static_cast<std::uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		}
	}

	Renderer3D::Renderer3D(Camera& camera) noexcept
		: camera(camera)
	{
		command_buffer = CommandBuffer::from_swapchain();

		data.sphere_model = Mesh::from_file("sphere.obj");

		auto image_count = Application::the().swapchain().get_image_count();
		create_renderpass();

		VkDeviceSize uniform_buffer_size = sizeof(UBO);

		for (std::size_t i = 0; i < image_count; i++) {
			data.uniforms.push_back(UniformBuffer(uniform_buffer_size, 0));
		}

		data.viking_room_texture = *AssetManager::ResourceCache::the().texture("viking_room");
		create_descriptor_set_layout();
		create_descriptor_pool();
		create_descriptor_sets();

		PipelineSpecification quad_spec {
			.shader = *AssetManager::ResourceCache::the().shader("main").value(),
			.debug_name = "Quad Pipeline",
			.render_pass = data.render_pass,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.vertex_layout = VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"),
				VertexBufferElement(ShaderDataType::Float4, "colour"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
		};
		data.quad_pipeline = std::make_unique<Pipeline>(quad_spec);
		data.quad_pipeline->invalidate();

		PipelineSpecification mesh_spec {
			.shader = *AssetManager::ResourceCache::the().shader("mesh").value(),
			.debug_name = "Mesh Pipeline",
			.render_pass = data.render_pass,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.vertex_layout = Layout::Default::vertex_attributes(),
			.ranges = Layout::Defaults::push_constants(),
		};
		data.mesh_pipeline = std::make_unique<Pipeline>(mesh_spec);
		data.mesh_pipeline->invalidate();

		PipelineSpecification line_spec { .shader = *AssetManager::ResourceCache::the().shader("line").value(),
			.debug_name = "Line Pipeline",
			.render_pass = data.render_pass,
			.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"), VertexBufferElement(ShaderDataType::Float4, "colour") },
			.line_width = 2.0f };
		data.line_pipeline = std::make_unique<Pipeline>(line_spec);
		data.line_pipeline->invalidate();

		data.quad_vertex_buffer = VertexBuffer::create(data.max_vertices * sizeof(QuadVertex));
		data.line_vertex_buffer = VertexBuffer::create(data.max_vertices * sizeof(LineVertex));

		std::vector<std::uint32_t> indices;
		indices.resize(RendererData::max_indices);
		std::uint32_t offset = 0;
		for (std::size_t i = 0; i < RendererData::max_indices; i += 6) {
			indices[i + 0] = 0 + offset;
			indices[i + 1] = 1 + offset;
			indices[i + 2] = 2 + offset;
			indices[i + 3] = 2 + offset;
			indices[i + 4] = 3 + offset;
			indices[i + 5] = 0 + offset;
			offset += 4;
		}
		data.quad_index_buffer = IndexBuffer::create(indices);

		indices.clear();
		indices.resize(RendererData::max_indices);
		for (std::uint32_t i = 0; i < RendererData::max_indices; i++) {
			indices[i] = i;
		}
		data.line_index_buffer = IndexBuffer::create(indices);
	};

	void Renderer3D::begin_scene()
	{
		reset_data(data);
		update_uniform_buffers();
	}

	void Renderer3D::reset_stats()
	{
		reset_data(data);
		data.draw_calls = 0;
	}

	void Renderer3D::quad(const glm::vec3& pos, const glm::vec4& colour, const glm::vec3& scale, float rotation)
	{
		static constexpr std::size_t quad_vertex_count = 4;
		static constexpr glm::vec2 texture_coordinates[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		static constexpr glm::vec4 quad_positions[]
			= { { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, 0.5f, 0.0f, 1.0f }, { -0.5f, 0.5f, 0.0f, 1.0f } };

		if (data.quad_indices_submitted >= RendererData::max_indices) {
			flush();
		}

		if (data.quad_vertices_submitted >= RendererData::max_vertices) {
			flush();
		}

		const auto transform = glm::translate(glm::mat4(1.0f), { pos.x, pos.y, pos.z })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3 { 1, 0, 0 }) * glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		for (std::size_t i = 0; i < quad_vertex_count; i++) {
			auto& vertex = data.quad_buffer[data.quad_vertices_submitted];
			vertex.position = transform * quad_positions[i];
			vertex.colour = colour;
			vertex.uvs = texture_coordinates[i];
			data.quad_vertices_submitted++;
		}

		data.quad_indices_submitted += 6;
	}

	void Renderer3D::mesh(const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Pipeline>& pipeline, const glm::vec3& pos,
		const glm::mat4& rotation_matrix, const glm::vec4& colour, const glm::vec3& scale)
	{
		auto transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), scale) * rotation_matrix;
		mesh->set_transform(std::move(transform));
		mesh->set_colour(colour);
		data.mesh[data.meshes_submitted] = mesh.get();
		data.mesh_pipeline_submit[data.meshes_submitted] = pipeline ? pipeline.get() : data.mesh_pipeline.get();
		data.meshes_submitted++;
	}

	void Renderer3D::line(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (data.line_indices_submitted >= RendererData::max_indices) {
			flush();
		}

		if (data.line_vertices_submitted >= RendererData::max_vertices) {
			flush();
		}

		auto& first_line_vertex = data.line_buffer[data.line_vertices_submitted++];
		first_line_vertex.position = glm::vec4(p0, 1.0f);
		first_line_vertex.colour = color;

		auto& second_line_vertex = data.line_buffer[data.line_vertices_submitted++];
		second_line_vertex.position = glm::vec4(p1, 1.0f);
		second_line_vertex.colour = color;

		data.line_indices_submitted += 2;
	}

	void Renderer3D::line(float size, const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (data.line_indices_submitted >= RendererData::max_indices) {
			flush();
		}

		if (data.line_vertices_submitted >= RendererData::max_vertices) {
			flush();
		}

		if (glm::epsilonEqual(size, 1.0f, 0.0001f)) {
			line(p0, p1, color);
		} else {
			// TODO: quad between two points here.
		}
	}

	void Renderer3D::flush()
	{
		// Flush ignores these for now...
	}

	void Renderer3D::end_scene()
	{
		command_buffer->begin();
		Renderer::begin_render_pass(command_buffer, data.render_pass, true);
		// CLEAR!
		Renderer::end_render_pass(command_buffer);
		Renderer::begin_render_pass(command_buffer, data.render_pass);

		if (data.quad_indices_submitted > 0) {
			std::uint32_t vertex_count = static_cast<std::uint32_t>(data.quad_vertices_submitted);

			std::uint32_t size = vertex_count * sizeof(QuadVertex);
			data.quad_vertex_buffer->set_data(data.quad_buffer.data(), size, 0);

			draw_quads();

			data.draw_calls++;
		}

		if (data.line_indices_submitted > 0) {
			std::uint32_t vertex_count = static_cast<std::uint32_t>(data.line_vertices_submitted);

			std::uint32_t size = vertex_count * sizeof(LineVertex);
			data.line_vertex_buffer->set_data(data.line_buffer.data(), size, 0);

			draw_lines();

			data.draw_calls++;
		}

		if (data.meshes_submitted > 0) {

			draw_meshes();

			data.draw_calls++;
		}

		Log::info("[Renderer3D] Draw calls: {}", data.draw_calls);

		Renderer::end_render_pass(command_buffer);
		command_buffer->submit();
	}

	void Renderer3D::draw_quads()
	{
		const auto& vb = data.quad_vertex_buffer;
		const auto& ib = data.quad_index_buffer;
		const auto& descriptor = data.descriptor_sets[Renderer::current_frame()];
		const auto& pipeline = data.quad_pipeline;
		const auto index_count = data.quad_indices_submitted;

		vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

		std::array<VkBuffer, 1> vbs { vb->get_vulkan_buffer() };
		VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(*command_buffer, 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(*command_buffer, ib->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

		if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}

		const auto count = static_cast<std::uint32_t>(index_count);
		const auto instances = static_cast<std::uint32_t>(index_count / 6);
		vkCmdDrawIndexed(*command_buffer, count, instances, 0, 0, 0);
	}

	void Renderer3D::draw_lines()
	{
		const auto& vb = data.line_vertex_buffer;
		const auto& ib = data.line_index_buffer;
		const auto& descriptor = data.descriptor_sets[Renderer::current_frame()];
		const auto& pipeline = data.line_pipeline;
		const auto index_count = data.line_indices_submitted;

		vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

		std::array<VkBuffer, 1> vbs { vb->get_vulkan_buffer() };
		VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(*command_buffer, 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(*command_buffer, ib->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

		if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}

		const auto line_width = pipeline->get_specification().line_width;
		vkCmdSetLineWidth(*command_buffer, line_width);

		const auto count = static_cast<std::uint32_t>(index_count);
		const auto instances = static_cast<std::uint32_t>(index_count / 2);
		vkCmdDrawIndexed(*command_buffer, count, instances, 0, 0, 0);
	}

	void Renderer3D::draw_meshes()
	{
		const VkDescriptorSet& descriptor = data.descriptor_sets[Renderer::current_frame()];

		for (std::uint32_t i = 0; i < data.meshes_submitted; i++) {
			const auto& mesh = data.mesh[i];

			const auto& vb = mesh->get_vertex_buffer();
			const auto& ib = mesh->get_index_buffer();
			const auto& pipeline = data.mesh_pipeline_submit[i];
			const auto& layout = pipeline->get_vulkan_pipeline_layout();
			const auto& vk_pipeline = pipeline->get_vulkan_pipeline();

			vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

			vkCmdBindDescriptorSets(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor, 0, nullptr);

			std::array<VkBuffer, 1> vbs { *mesh->get_vertex_buffer() };
			VkDeviceSize offsets { 0 };
			vkCmdBindVertexBuffers(*command_buffer, 0, 1, vbs.data(), &offsets);

			vkCmdBindIndexBuffer(*command_buffer, *mesh->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);

			RendererTransform rt {};
			const auto mesh_transform = mesh->get_transform();
			const auto mesh_colour = mesh->get_colour();

			if (mesh_transform) {
				rt.transform = *mesh_transform;
			}
			if (mesh_colour) {
				rt.colour = *mesh_colour;
			}

			update_uniform_buffers(rt.transform);
			vkCmdPushConstants(*command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(RendererTransform), &rt);

			vkCmdDrawIndexed(*command_buffer, static_cast<std::uint32_t>(mesh->get_index_count()), 1, 0, 0, 0);
		}
	}

	void Renderer3D::update_uniform_buffers(const std::optional<glm::mat4>& model)
	{
		const auto image_index = Application::the().swapchain().frame();

		UBO ubo {};
		ubo.projection = camera.get_projection_matrix();
		ubo.view = camera.get_view_matrix();
		if (model) {
			ubo.model = *model;
		} else {
			ubo.model = default_model;
		}
		ubo.view_projection = camera.get_view_projection();

		data.uniforms[image_index].set_data(&ubo, sizeof(ubo), 0);
	}

	void Renderer3D::destroy()
	{
		auto device = GraphicsContext::the().device();

		vkDestroyRenderPass(device, data.render_pass, nullptr);

		for (std::size_t i = 0; i < Application::the().swapchain().get_image_count(); i++) {
			data.uniforms[i].destroy();
		}

		vkDestroyDescriptorPool(device, data.descriptor_pool, nullptr);
		vkDestroyDescriptorSetLayout(device, data.descriptor_set_layout, nullptr);

		command_buffer->destroy();

		data.line_vertex_buffer->destroy();
		data.quad_vertex_buffer->destroy();
		data.line_index_buffer->destroy();
		data.quad_index_buffer->destroy();

		data.quad_pipeline->destroy();
		data.line_pipeline->destroy();
		data.mesh_pipeline->destroy();

		data.sphere_model->destroy();
	}

	const VkRenderPass& Renderer3D::get_render_pass() const { return data.render_pass; }

	void Renderer3D::set_camera(Camera& cam) { camera = cam; }
} // namespace Alabaster
