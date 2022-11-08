#include "av_pch.hpp"

#include "graphics/Renderer3D.hpp"

#include "core/Application.hpp"
#include "core/Window.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/UniformBuffer.hpp"
#include "graphics/VertexBuffer.hpp"

#include <imgui.h>
#include <vulkan/vulkan.h>

#define USE_CAMERA

namespace Alabaster {

	static void reset_data(RendererData& to_reset)
	{
		to_reset.quad_buffer_ptr = &to_reset.quad_buffer[0];
		to_reset.indices_submitted = 0;
		to_reset.vertices_submitted = 0;
		to_reset.line_buffer_ptr = &to_reset.line_buffer[0];
		to_reset.line_indices_submitted = 0;
		to_reset.line_vertices_submitted = 0;
	}

	void Renderer3D::create_renderpass()
	{
		VkAttachmentDescription color_attachment {};
		color_attachment.format = Application::the().get_window()->get_swapchain()->get_format();
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { ubo_layout_binding };
		VkDescriptorSetLayoutCreateInfo layout_info {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		vk_check(vkCreateDescriptorSetLayout(GraphicsContext::the().device(), &layout_info, nullptr, &data.descriptor_set_layout));
	}

	void Renderer3D::create_descriptor_pool()
	{
		std::array<VkDescriptorPoolSize, 1> pool_sizes {};
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = static_cast<uint32_t>(Application::the().swapchain().get_image_count());

		VkDescriptorPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(Application::the().swapchain().get_image_count());

		if (vkCreateDescriptorPool(GraphicsContext::the().device(), &pool_info, nullptr, &data.descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void Renderer3D::create_descriptor_sets()
	{
		std::vector<VkDescriptorSetLayout> layouts(Application::the().swapchain().get_image_count(), data.descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = data.descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(Application::the().swapchain().get_image_count());
		alloc_info.pSetLayouts = layouts.data();

		data.descriptor_sets.resize(Application::the().swapchain().get_image_count());
		vk_check(vkAllocateDescriptorSets(GraphicsContext::the().device(), &alloc_info, data.descriptor_sets.data()));

		for (size_t i = 0; i < Application::the().swapchain().get_image_count(); i++) {
			VkDescriptorBufferInfo buffer_info {};
			buffer_info.buffer = data.uniform_buffers[i];
			buffer_info.offset = 0;
			buffer_info.range = sizeof(UBO);

			std::array<VkWriteDescriptorSet, 1> descriptor_writes {};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = data.descriptor_sets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			vkUpdateDescriptorSets(
				GraphicsContext::the().device(), static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		}
	}

	uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(GraphicsContext::the().physical_device(), &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			const auto filter = (type_filter & (1 << i));
			if (filter && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
	{
		VkBufferCreateInfo buffer_info {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(GraphicsContext::the().device(), &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(GraphicsContext::the().device(), buffer, &mem_requirements);

		VkMemoryAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

		vk_check(vkAllocateMemory(GraphicsContext::the().device(), &alloc_info, nullptr, &memory));

		vkBindBufferMemory(GraphicsContext::the().device(), buffer, memory, 0);
	}

	Renderer3D::Renderer3D(SimpleCamera& camera) noexcept
		: camera(camera)
		, command_buffer("Swapchain")
	{
		auto image_count = Application::the().swapchain().get_image_count();
		create_renderpass();

		VkDeviceSize uniform_buffer_size = sizeof(UBO);

		data.uniform_buffers.resize(image_count);
		data.uniform_buffers_memory.resize(image_count);

		for (size_t i = 0; i < image_count; i++) {
			create_buffer(uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data.uniform_buffers[i], data.uniform_buffers_memory[i]);
		}

		create_descriptor_set_layout();
		create_descriptor_pool();
		create_descriptor_sets();

		PipelineSpecification spec {
			.shader = Shader("app/resources/shaders/main"),
			.debug_name = "Quad Pipeline",
			.render_pass = data.render_pass,
			.wireframe = false,
			.backface_culling = false,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.depth_test = false,
			.depth_write = false,
			.vertex_layout = VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"),
				VertexBufferElement(ShaderDataType::Float4, "colour"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.instance_layout = {},
		};
		data.quad_pipeline = std::make_unique<Pipeline>(spec);
		data.quad_pipeline->invalidate();

		PipelineSpecification line { .shader = Shader("app/resources/shaders/line"),
			.debug_name = "Line Pipeline",
			.render_pass = data.render_pass,
			.wireframe = false,
			.backface_culling = false,
			.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
			.depth_test = false,
			.depth_write = false,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"), VertexBufferElement(ShaderDataType::Float4, "colour") },
			.instance_layout = {},
			.line_width = 2.0f };
		data.line_pipeline = std::make_unique<Pipeline>(line);
		data.line_pipeline->invalidate();

		data.quad_vertex_buffer = VertexBuffer::create(data.max_vertices * sizeof(QuadVertex));
		data.line_vertex_buffer = VertexBuffer::create(data.max_vertices * sizeof(LineVertex));

		std::vector<uint32_t> indices;
		indices.resize(RendererData::max_indices);
		uint32_t offset = 0;
		for (size_t i = 0; i < RendererData::max_indices; i += 6) {
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
		for (uint32_t i = 0; i < RendererData::max_indices; i++) {
			indices[i] = i;
		}
		data.line_index_buffer = IndexBuffer::create(indices);

		data.quad_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		data.quad_positions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		data.quad_positions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
		data.quad_positions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
	};

	void Renderer3D::begin_scene()
	{
		Renderer::submit([this] {
			reset_data(data);
			update_uniform_buffers();
		});
	}

	void Renderer3D::reset_stats()
	{
		Renderer::submit([this] {
			reset_data(data);
			data.draw_calls = 0;
		});
	}

	void Renderer3D::quad(const glm::vec4& pos, const glm::vec4& colour, const glm::vec3& scale)
	{
		static constexpr size_t quad_vertex_count = 4;
		static constexpr glm::vec2 texture_coordinates[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		if (data.indices_submitted >= RendererData::max_indices) {
			flush();
		}

		if (data.vertices_submitted >= RendererData::max_vertices) {
			flush();
		}

		const auto transform = glm::translate(glm::mat4(1.0f), { pos.x, pos.y, pos.z }) * glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		for (size_t i = 0; i < quad_vertex_count; i++) {
			auto& vertex = data.quad_buffer[data.vertices_submitted];
			vertex.position = transform * data.quad_positions[i];
			vertex.colour = colour;
			vertex.uvs = texture_coordinates[i];
			data.vertices_submitted++;
		}

		data.indices_submitted += 6;
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

	void Renderer3D::flush()
	{
		// Flush ignores these for now...
	}

	void Renderer3D::end_scene()
	{
		command_buffer.begin();
		Renderer::begin_render_pass(command_buffer, data.render_pass);

		if (data.indices_submitted > 0) {
			uint32_t vertex_count = static_cast<uint32_t>(data.vertices_submitted);

			uint32_t size = vertex_count * sizeof(QuadVertex);
			data.quad_vertex_buffer.reset(new VertexBuffer(data.quad_buffer.data(), size));

			draw_quads();

			data.draw_calls++;
		}

		if (data.line_indices_submitted > 0) {
			uint32_t vertex_count = static_cast<uint32_t>(data.line_vertices_submitted);

			uint32_t size = vertex_count * sizeof(LineVertex);
			data.line_vertex_buffer.reset(new VertexBuffer(data.line_buffer.data(), size));

			draw_lines();

			data.draw_calls++;
		}

		Log::info("[Renderer3D] Draw calls: {}", data.draw_calls);

		Renderer::end_render_pass(command_buffer);
		command_buffer.end();
		command_buffer.submit();

		reset_data(data);
	}

	void Renderer3D::draw_quads()
	{
		Renderer::submit(
			[&buffer = command_buffer, index_count = data.indices_submitted, &pipeline = data.quad_pipeline, &vb = data.quad_vertex_buffer,
				&ib = data.quad_index_buffer, &descriptor = data.descriptor_sets] {
				vkCmdBindPipeline(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

				std::array<VkBuffer, 1> vbs { vb->get_vulkan_buffer() };
				VkDeviceSize offsets { 0 };
				vkCmdBindVertexBuffers(*buffer, 0, 1, vbs.data(), &offsets);

				vkCmdBindIndexBuffer(*buffer, ib->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

				if (pipeline->get_vulkan_pipeline_layout()) {
					vkCmdBindDescriptorSets(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1,
						&descriptor[Renderer::current_frame()], 0, nullptr);
				}

				const auto count = static_cast<uint32_t>(index_count);
				const auto instances = static_cast<uint32_t>(index_count / 6);
				vkCmdDrawIndexed(*buffer, count, instances, 0, 0, 0);
			},
			"Draw quads");
	}

	void Renderer3D::draw_lines()
	{
		Renderer::submit(
			[&buffer = command_buffer, index_count = data.line_indices_submitted, &pipeline = data.line_pipeline, &vb = data.line_vertex_buffer,
				&ib = data.line_index_buffer, &descriptor = data.descriptor_sets] {
				vkCmdBindPipeline(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

				std::array<VkBuffer, 1> vbs { vb->get_vulkan_buffer() };
				VkDeviceSize offsets { 0 };
				vkCmdBindVertexBuffers(*buffer, 0, 1, vbs.data(), &offsets);

				vkCmdBindIndexBuffer(*buffer, ib->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

				if (pipeline->get_vulkan_pipeline_layout()) {
					vkCmdBindDescriptorSets(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1,
						&descriptor[Renderer::current_frame()], 0, nullptr);
				}

				const auto line_width = pipeline->get_specification().line_width;
				vkCmdSetLineWidth(*buffer, line_width);

				const auto count = static_cast<uint32_t>(index_count);
				const auto instances = static_cast<uint32_t>(index_count / 2);
				vkCmdDrawIndexed(*buffer, count, instances, 0, 0, 0);
			},
			"Draw lines");
	}

	void Renderer3D::mesh(const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Pipeline>& pipe, const glm::vec4& pos, const glm::vec4& colour,
		const glm::vec3& scale)
	{
		Renderer::submit(
			[&buffer = command_buffer, &mesh = mesh, &descriptor = data.descriptor_sets, &pipeline = pipe] {
				vkCmdBindPipeline(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

				std::array<VkBuffer, 1> vbs { *mesh->get_vertex_buffer() };
				VkDeviceSize offsets { 0 };
				vkCmdBindVertexBuffers(*buffer, 0, 1, vbs.data(), &offsets);

				vkCmdBindIndexBuffer(*buffer, *mesh->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);

				if (pipeline->get_vulkan_pipeline_layout()) {
					vkCmdBindDescriptorSets(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1,
						&descriptor[Renderer::current_frame()], 0, nullptr);
				}

				vkCmdDrawIndexed(*buffer, mesh->get_index_buffer().count(), 1, 0, 0, 0);
			},
			"Draw meshes");
	}

	void Renderer3D::update_uniform_buffers()
	{
		Renderer::submit([this] {
			auto image_index = Application::the().swapchain().frame();

			UBO ubo {};
			ubo.projection = camera.get_projection_matrix();
			ubo.view = camera.get_view_matrix();
			// ubo.view = glm::lookAt(glm::vec3(0, 2, 2), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			const auto ar = Application::the().swapchain().aspect_ratio();
			// ubo.projection = glm::perspective(glm::radians(45.0f), 0.1f, ar, 10.0f);
			ubo.view_projection = ubo.projection * ubo.view;
			ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			void* mapped;
			vkMapMemory(GraphicsContext::the().device(), data.uniform_buffers_memory[image_index], 0, sizeof(ubo), 0, &mapped);
			std::memcpy(mapped, &ubo, sizeof(ubo));
			vkUnmapMemory(GraphicsContext::the().device(), data.uniform_buffers_memory[image_index]);
		});
	}

} // namespace Alabaster
