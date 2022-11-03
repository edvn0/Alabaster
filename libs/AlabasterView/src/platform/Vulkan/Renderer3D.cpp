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
#include "graphics/VertexBuffer.hpp"

#include <imgui.h>
#include <vulkan/vulkan.h>

// #define USE_CAMERA

namespace Alabaster {

	static void reset_data(RendererData& to_reset)
	{
		to_reset.quad_buffer_ptr = &to_reset.quad_buffer[0];
		to_reset.indices_submitted = 0;
		to_reset.vertices_submitted = 0;
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

	Renderer3D::Renderer3D(EditorCamera& camera) noexcept
		: camera(camera)
	{
		auto image_count = Application::the().swapchain().get_image_count();

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

		std::array<QuadVertex, 4> quad_vertices {};
		std::array<uint32_t, 6> quad_indices {};

		quad_vertices[0] = QuadVertex { .position = { -0.5, 0.5, 0, 1 }, .colour = { 1, 0, 0, 1 }, .uvs = { 0, 0 } };
		quad_vertices[1] = QuadVertex { .position = { 0.5, 0.5, 0, 1 }, .colour = { 1, 0, 0, 1 }, .uvs = { 1, 0 } };
		quad_vertices[2] = QuadVertex { .position = { 0.5, -0.5, 0, 1 }, .colour = { 1, 1, 0, 1 }, .uvs = { 1, 1 } };
		quad_vertices[3] = QuadVertex { .position = { -0.5, -0.5, 0, 1 }, .colour = { 1, 0, 1, 1 }, .uvs = { 0, 1 } };

		PipelineSpecification spec {
			.shader = Shader("app/resources/shaders/main"),
			.debug_name = "Test",
			.render_pass = Application::the().swapchain().get_render_pass(),
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

		data.quad_vertex_buffer = VertexBuffer::create(data.max_vertices * sizeof(QuadVertex));

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
		data.quad_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		data.quad_positions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		data.quad_positions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
		data.quad_positions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
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

	void Renderer3D::mesh(const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Pipeline>& pipe, const glm::vec4& pos, const glm::vec4& colour,
		const glm::vec3& scale)
	{
		Renderer::submit([&mesh = mesh, &render_pass = data.render_pass, &descriptor = data.descriptor_sets, &pipeline = pipe] {
			const auto& swapchain = Application::the().swapchain();
			VkCommandBufferBeginInfo command_buffer_begin_info = {};
			command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pNext = nullptr;

			const auto& buffer = swapchain.get_current_drawbuffer();
			vk_check(vkBeginCommandBuffer(buffer, &command_buffer_begin_info));

			auto extent = swapchain.swapchain_extent();

			VkRenderPassBeginInfo render_pass_info {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = swapchain.get_render_pass();
			render_pass_info.framebuffer = swapchain.get_current_framebuffer();
			render_pass_info.renderArea.offset = { 0, 0 };
			render_pass_info.renderArea.extent = extent;

			std::array<VkClearValue, 2> clear_values {};
			clear_values[0].color = { 0, 0, 0, 0 };
			clear_values[1].depthStencil = { .depth = -1.0f, .stencil = 0 };

			render_pass_info.clearValueCount = clear_values.size();
			render_pass_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

			VkViewport viewport {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(extent.width);
			viewport.height = static_cast<float>(extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(buffer, 0, 1, &viewport);

			VkRect2D scissor {};
			scissor.offset = { 0, 0 };
			scissor.extent = extent;
			vkCmdSetScissor(buffer, 0, 1, &scissor);

			std::array<VkBuffer, 1> vbs {};
			vbs[0] = *mesh->get_vertex_buffer();
			VkDeviceSize offsets { 0 };
			vkCmdBindVertexBuffers(buffer, 0, 1, vbs.data(), &offsets);

			vkCmdBindIndexBuffer(buffer, *mesh->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);

			if (pipeline->get_vulkan_pipeline_layout()) {
				vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1,
					&descriptor[swapchain.frame()], 0, nullptr);
			}

			vkCmdDrawIndexed(buffer, mesh->get_index_buffer().count(), 1, 0, 0, 0);

			vkCmdEndRenderPass(buffer);

			vkEndCommandBuffer(buffer);
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

		const auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(pos)) * glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f });

		for (size_t i = 0; i < quad_vertex_count; i++) {
			auto& vertex = data.quad_buffer[data.vertices_submitted];
			vertex.position = transform * data.quad_positions[i];
			vertex.colour = colour;
			vertex.uvs = texture_coordinates[i];
			data.vertices_submitted++;
		}

		data.indices_submitted += 6;
	}

	void Renderer3D::flush()
	{
		if (data.indices_submitted > 0) {
			Log::info("[Renderer3D] Draw calls: {}", data.draw_calls);
			uint32_t vertex_count = data.vertices_submitted;

			uint32_t size = vertex_count * sizeof(QuadVertex);
			data.quad_vertex_buffer.reset(new VertexBuffer(data.quad_buffer.data(), size));

			draw_quads();

			data.draw_calls++;
		}

		reset_data(data);
	}

	void Renderer3D::end_scene() { flush(); }

	void Renderer3D::draw_quads()
	{
		Renderer::submit([index_count = data.indices_submitted, &pipeline = data.quad_pipeline, &vb = data.quad_vertex_buffer,
							 &ib = data.quad_index_buffer, &descriptor = data.descriptor_sets] {
			const auto& swapchain = Application::the().swapchain();
			VkCommandBufferBeginInfo command_buffer_begin_info = {};
			command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			command_buffer_begin_info.pNext = nullptr;

			const auto& buffer = swapchain.get_current_drawbuffer();
			vk_check(vkBeginCommandBuffer(buffer, &command_buffer_begin_info));

			auto extent = swapchain.swapchain_extent();

			VkRenderPassBeginInfo render_pass_info {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = swapchain.get_render_pass();
			render_pass_info.framebuffer = swapchain.get_current_framebuffer();
			render_pass_info.renderArea.offset = { 0, 0 };
			render_pass_info.renderArea.extent = extent;

			std::array<VkClearValue, 2> clear_values {};
			clear_values[0].color = { 0, 0, 0, 0 };
			clear_values[1].depthStencil = { .depth = -1.0f, .stencil = 0 };

			render_pass_info.clearValueCount = clear_values.size();
			render_pass_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

			VkViewport viewport {};
			viewport.x = 0.0f;
			viewport.y = static_cast<float>(extent.height);
			viewport.width = static_cast<float>(extent.width);
			viewport.height = -static_cast<float>(extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(buffer, 0, 1, &viewport);

			VkRect2D scissor {};
			scissor.offset = { 0, 0 };
			scissor.extent = extent;
			vkCmdSetScissor(buffer, 0, 1, &scissor);

			std::array<VkBuffer, 1> vbs {};
			vbs[0] = vb->get_vulkan_buffer();
			VkDeviceSize offsets { 0 };
			vkCmdBindVertexBuffers(buffer, 0, 1, vbs.data(), &offsets);

			vkCmdBindIndexBuffer(buffer, ib->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

			if (pipeline->get_vulkan_pipeline_layout()) {
				vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1,
					&descriptor[swapchain.frame()], 0, nullptr);
			}

			vkCmdDrawIndexed(buffer, index_count, index_count / 6, 0, 0, 0);

			vkCmdEndRenderPass(buffer);

			vkEndCommandBuffer(buffer);
		});
	}

	void Renderer3D::update_uniform_buffers()
	{
		auto image_index = Application::the().swapchain().frame();
		static auto start_time = std::chrono::high_resolution_clock::now();

		auto current_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
		UBO ubo {};
#ifdef USE_CAMERA
		ubo.projection = camera.get_projection_matrix();
		ubo.view = camera.get_view_matrix();
		ubo.view_projection = camera.get_view_projection();
#else
		ubo.view = glm::lookAt(glm::vec3(0, -2, -2), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.projection = glm::perspective(glm::radians(45.0f),
			static_cast<float>(Application::the().swapchain().get_width()) / static_cast<float>(Application::the().swapchain().get_height()), 0.1f,
			10.0f);
		ubo.view_projection = ubo.projection * ubo.view;
#endif
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		void* mapped;
		vkMapMemory(GraphicsContext::the().device(), data.uniform_buffers_memory[image_index], 0, sizeof(ubo), 0, &mapped);
		std::memcpy(mapped, &ubo, sizeof(ubo));
		vkUnmapMemory(GraphicsContext::the().device(), data.uniform_buffers_memory[image_index]);
	}

} // namespace Alabaster
