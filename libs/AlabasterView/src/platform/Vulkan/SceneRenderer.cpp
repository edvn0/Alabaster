#include "av_pch.hpp"

#include "graphics/SceneRenderer.hpp"

#include "core/Application.hpp"
#include "core/Window.hpp"
#include "graphics/Camera.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/VertexBuffer.hpp"

#include <vulkan/vulkan.h>

namespace Alabaster {

	void SceneRenderer::create_renderpass()
	{
		VkAttachmentDescription colour_attachment {};
		colour_attachment.format = Application::the().get_window()->get_swapchain()->get_format();
		colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colour_attachment_ref {};
		colour_attachment_ref.attachment = 0;
		colour_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colour_attachment_ref;

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
		render_pass_info.pAttachments = &colour_attachment;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &render_pass_info, nullptr, &render_pass));
	}

	void SceneRenderer::create_descriptor_set_layout()
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

		if (vkCreateDescriptorSetLayout(GraphicsContext::the().device(), &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void SceneRenderer::create_descriptor_pool()
	{
		std::array<VkDescriptorPoolSize, 1> pool_sizes {};
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = static_cast<uint32_t>(Application::the().swapchain().get_image_count());

		VkDescriptorPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(Application::the().swapchain().get_image_count());

		if (vkCreateDescriptorPool(GraphicsContext::the().device(), &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void SceneRenderer::create_descriptor_sets()
	{
		std::vector<VkDescriptorSetLayout> layouts(Application::the().swapchain().get_image_count(), descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(Application::the().swapchain().get_image_count());
		alloc_info.pSetLayouts = layouts.data();

		descriptor_sets.resize(Application::the().swapchain().get_image_count());
		if (vkAllocateDescriptorSets(GraphicsContext::the().device(), &alloc_info, descriptor_sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < Application::the().swapchain().get_image_count(); i++) {
			VkDescriptorBufferInfo buffer_info {};
			buffer_info.buffer = uniform_buffers[i];
			buffer_info.offset = 0;
			buffer_info.range = sizeof(UBO);

			std::array<VkWriteDescriptorSet, 1> descriptor_writes {};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = descriptor_sets[i];
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

	void SceneRenderer::init()
	{
		auto image_count = Application::the().swapchain().get_image_count();

		VkDeviceSize buffer_size = sizeof(UBO);

		uniform_buffers.resize(image_count);
		uniform_buffers_memory.resize(image_count);

		for (size_t i = 0; i < image_count; i++) {
			create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniform_buffers[i], uniform_buffers_memory[i]);
		}

		create_descriptor_set_layout();
		create_descriptor_pool();
		create_descriptor_sets();
		create_renderpass();
	}

	void SceneRenderer::basic_mesh(
		const std::unique_ptr<Mesh>& basic_mesh, const std::unique_ptr<Camera>& camera, const std::unique_ptr<Pipeline>& pipeline)
	{
		const auto& editor_camera = as<EditorCamera>(camera);
		update_uniform_buffers(editor_camera.get_view_matrix(), editor_camera.get_projection_matrix());

		Renderer::submit(
			[this, &basic_mesh, &pipeline] {
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
				render_pass_info.renderPass = render_pass;
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
				vbs[0] = *basic_mesh->get_vertex_buffer();
				VkDeviceSize offsets { 0 };
				vkCmdBindVertexBuffers(buffer, 0, 1, vbs.data(), &offsets);

				vkCmdBindIndexBuffer(buffer, *basic_mesh->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);

				if (pipeline->get_vulkan_pipeline_layout()) {
					vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1,
						&descriptor_sets[swapchain.frame()], 0, nullptr);
				}

				vkCmdDrawIndexed(buffer, basic_mesh->get_index_buffer().count(), 1, 0, 0, 0);

				vkCmdEndRenderPass(buffer);

				vkEndCommandBuffer(buffer);
			},
			"BasicMesh");
	}

	void SceneRenderer::update_uniform_buffers(const glm::mat4& view, const glm::mat4& projection)
	{
		auto image_index = Application::the().swapchain().frame();

		UBO ubo {};
		ubo.model = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.projection = projection;
		ubo.view = view;
		ubo.view_projection = projection * view;

		void* data;
		vkMapMemory(GraphicsContext::the().device(), uniform_buffers_memory[image_index], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(GraphicsContext::the().device(), uniform_buffers_memory[image_index]);
	}

} // namespace Alabaster
