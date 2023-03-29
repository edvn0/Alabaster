#include "av_pch.hpp"

#include "graphics/Renderer3D.hpp"

#include "AssetManager.hpp"
#include "core/Application.hpp"
#include "core/Window.hpp"
#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantRange.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBufferLayout.hpp"

#include <memory>
#include <vulkan/vulkan.h>

namespace Alabaster {

	using namespace std::string_view_literals;

	static constexpr auto default_model = glm::mat4 { 1.0f };

	static void reset_data(RendererData& to_reset)
	{
		to_reset.quad_indices_submitted = 0;
		to_reset.quad_vertices_submitted = 0;
		to_reset.line_indices_submitted = 0;
		to_reset.line_vertices_submitted = 0;
		to_reset.meshes_submitted = 0;
		to_reset.mesh.fill(nullptr);
		to_reset.mesh_pipeline_submit.fill(nullptr);
		to_reset.mesh_transform.fill({});
		to_reset.mesh_colour.fill({});
		to_reset.push_constant = {};
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
		pool_sizes[0].descriptorCount = Application::the().swapchain().get_image_count();

		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[1].descriptorCount = Application::the().swapchain().get_image_count();

		VkDescriptorPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = Application::the().swapchain().get_image_count();

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
		alloc_info.descriptorSetCount = image_count;
		alloc_info.pSetLayouts = layouts.data();

		data.descriptor_sets.resize(image_count);
		vk_check(vkAllocateDescriptorSets(GraphicsContext::the().device(), &alloc_info, data.descriptor_sets.data()));

		const auto& image_info = AssetManager::the().texture("viking_room.png")->get_descriptor_info();
		for (std::size_t i = 0; i < image_count; i++) {
			VkDescriptorBufferInfo buffer_info {};
			buffer_info.buffer = data.uniforms[i]->get_buffer();
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

	Renderer3D::Renderer3D(const std::shared_ptr<Camera>& cam) noexcept
		: camera(cam)
	{
		const auto&& [w, h] = Application::the().get_window()->size();
		FramebufferSpecification fbs;
		fbs.width = w;
		fbs.height = h;
		fbs.attachments = { ImageFormat::RGBA, ImageFormat::DEPTH32F };
		fbs.samples = 1;
		fbs.clear_colour = { 0.0f, 0.0f, 0.0f, 1.0f };
		fbs.debug_name = "Geometry";
		fbs.clear_depth_on_load = true;
		data.framebuffer = Framebuffer::create(fbs);

		auto image_count = Application::the().swapchain().get_image_count();

		VkDeviceSize uniform_buffer_size = sizeof(UBO);

		for (std::size_t i = 0; i < image_count; i++) {
			data.uniforms.emplace_back(std::make_unique<UniformBuffer>(uniform_buffer_size, 0));
		}

		create_descriptor_set_layout();
		create_descriptor_pool();
		create_descriptor_sets();

		// QUAD STUFF

		PipelineSpecification quad_spec { .shader = AssetManager::the().shader("quad_light"),
			.debug_name = "Quad Pipeline",
			.render_pass = data.framebuffer->get_renderpass(),
			.topology = Topology::TriangleList,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
				VertexBufferElement(ShaderDataType::Float3, "normals"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) } };
		data.pipelines.try_emplace("quad"sv, std::make_unique<Pipeline>(quad_spec));

		data.quad_vertex_buffer = VertexBuffer::create(RendererData::max_vertices * sizeof(QuadVertex));
		data.line_vertex_buffer = VertexBuffer::create(RendererData::max_vertices * sizeof(LineVertex));

		std::vector<std::uint32_t> quad_indices;
		quad_indices.resize(RendererData::max_indices);
		std::uint32_t offset = 0;
		for (std::size_t i = 0; i < RendererData::max_indices; i += 6) {
			quad_indices[i + 0] = 0 + offset;
			quad_indices[i + 1] = 1 + offset;
			quad_indices[i + 2] = 2 + offset;
			quad_indices[i + 3] = 2 + offset;
			quad_indices[i + 4] = 3 + offset;
			quad_indices[i + 5] = 0 + offset;
			offset += 4;
		}
		data.quad_index_buffer = IndexBuffer::create(quad_indices);

		// END QUAD STUFF

		PipelineSpecification mesh_spec {
			.shader = AssetManager::the().shader("mesh_light"),
			.debug_name = "Mesh Pipeline",
			.render_pass = data.framebuffer->get_renderpass(),
			.topology = Topology::TriangleList,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
				VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
				VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) },
		};
		data.pipelines.try_emplace("mesh"sv, std::make_unique<Pipeline>(mesh_spec));

		PipelineSpecification line_spec { .shader = AssetManager::the().shader("line"),
			.debug_name = "Line Pipeline",
			.render_pass = data.framebuffer->get_renderpass(),
			.topology = Topology::LineList,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"), VertexBufferElement(ShaderDataType::Float4, "colour") },
			.line_width = 5.0f };
		data.pipelines.try_emplace("line"sv, std::make_unique<Pipeline>(line_spec));

		std::vector<std::uint32_t> line_indices;
		line_indices.resize(RendererData::max_indices);
		for (std::uint32_t i = 0; i < RendererData::max_indices; i++) {
			line_indices[i] = i;
		}
		data.line_index_buffer = IndexBuffer::create(line_indices);

		invalidate_pipelines();
	}

	void Renderer3D::invalidate_pipelines()
	{
		for (const auto& [k, pipe] : data.pipelines) {
			pipe->invalidate();
		}
	}

	void Renderer3D::begin_scene()
	{
		reset_data(data);
		update_uniform_buffers();
		data.push_constant = PC();
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
		static constexpr glm::vec4 quad_normal = glm::vec4 { 0, 0, 1, 0 };

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
			vertex.normals = transform * quad_normal;
			vertex.uvs = texture_coordinates[i];
			data.quad_vertices_submitted++;
		}

		data.quad_indices_submitted += 6;
	}

	void Renderer3D::quad(const glm::mat4& transform, const glm::vec4& colour)
	{
		static constexpr std::size_t quad_vertex_count = 4;
		static constexpr std::array<glm::vec2, 4> texture_coordinates = { glm::vec2 { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		static constexpr std::array<glm::vec4, 4> quad_positions
			= { glm::vec4 { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, 0.5f, 0.0f, 1.0f }, { -0.5f, 0.5f, 0.0f, 1.0f } };
		static constexpr glm::vec4 quad_normal = glm::vec4 { 0, 0, 1, 0 };

		if (data.quad_indices_submitted >= RendererData::max_indices) {
			flush();
		}

		if (data.quad_vertices_submitted >= RendererData::max_vertices) {
			flush();
		}

		for (std::size_t i = 0; i < quad_vertex_count; i++) {
			auto& vertex = data.quad_buffer[data.quad_vertices_submitted];
			vertex.position = transform * quad_positions[i];
			vertex.colour = colour;
			vertex.normals = transform * quad_normal;
			vertex.uvs = texture_coordinates[i];
			data.quad_vertices_submitted++;
		}

		data.quad_indices_submitted += 6;
	}

	void Renderer3D::mesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Pipeline>& pipeline, const glm::vec3& pos,
		const glm::mat4& rotation_matrix, const glm::vec4& colour, const glm::vec3& scale)
	{
		const auto transform = glm::translate(glm::mat4(1.0f), pos) * rotation_matrix * glm::scale(glm::mat4(1.0f), scale);
		this->mesh(mesh, std::move(transform), pipeline, colour);
	}

	void Renderer3D::mesh(const std::shared_ptr<Mesh>& mesh, const glm::vec3& pos, const glm::vec4& colour, const glm::vec3& scale)
	{
		this->mesh(mesh, nullptr, pos, glm::mat4 { 1.0f }, colour, scale);
	}

	void Renderer3D::mesh(
		const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, const std::shared_ptr<Pipeline>& pipeline, const glm::vec4& colour)
	{
		data.mesh_transform[data.meshes_submitted] = transform;
		data.mesh_colour[data.meshes_submitted] = colour;
		data.mesh[data.meshes_submitted] = mesh.get();
		data.mesh_pipeline_submit[data.meshes_submitted] = pipeline ? pipeline.get() : data.pipelines["mesh"sv].get();
		data.meshes_submitted++;
	}

	void Renderer3D::mesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, const glm::vec4& colour)
	{
		data.mesh_transform[data.meshes_submitted] = transform;
		data.mesh_colour[data.meshes_submitted] = colour;
		data.mesh[data.meshes_submitted] = mesh.get();
		data.mesh_pipeline_submit[data.meshes_submitted] = data.pipelines["mesh"sv].get();
		data.meshes_submitted++;
	}

	void Renderer3D::line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color)
	{
		if (data.line_indices_submitted >= RendererData::max_indices) {
			flush();
		}

		if (data.line_vertices_submitted >= RendererData::max_vertices) {
			flush();
		}

		{
			auto& [position, colour] = data.line_buffer[data.line_vertices_submitted++];
			position = glm::vec4(from, 1.0f);
			colour = color;
		}

		{
			auto& [position, colour] = data.line_buffer[data.line_vertices_submitted++];
			position = glm::vec4(to, 1.0f);
			colour = color;
		}

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

	void Renderer3D::text(std::string, glm::vec3, float) { }

	void Renderer3D::set_light_data(const glm::vec4& light_position, const glm::vec4& colour, float ambience)
	{
		data.push_constant.light_position = light_position;
		data.push_constant.light_colour = colour;
		data.push_constant.light_ambience = glm::vec4 { ambience };
	}

	void Renderer3D::flush()
	{

		// Flush ignores these for now...
	}

	void Renderer3D::end_scene(const CommandBuffer& command_buffer, const std::shared_ptr<Framebuffer>& target)
	{
		Renderer::begin_render_pass(command_buffer, target);
		if (data.quad_indices_submitted > 0) {
			const auto vertex_count = data.quad_vertices_submitted;

			const auto size = vertex_count * sizeof(QuadVertex);
			data.quad_vertex_buffer->set_data(data.quad_buffer.data(), size, 0);

			draw_quads(command_buffer);

			data.draw_calls++;
		}

		if (data.line_indices_submitted > 0) {
			const auto vertex_count = data.line_vertices_submitted;

			const auto size = vertex_count * sizeof(LineVertex);
			data.line_vertex_buffer->set_data(data.line_buffer.data(), size, 0);

			draw_lines(command_buffer);

			data.draw_calls++;
		}

		if (data.meshes_submitted > 0) {
			draw_meshes(command_buffer);
		}

		Renderer::end_render_pass(command_buffer);
	}

	void Renderer3D::end_scene(const CommandBuffer& command_buffer)
	{
		Renderer::begin_render_pass(command_buffer, data.framebuffer);
		if (data.quad_indices_submitted > 0) {
			const auto vertex_count = data.quad_vertices_submitted;

			const auto size = vertex_count * sizeof(QuadVertex);
			data.quad_vertex_buffer->set_data(data.quad_buffer.data(), size, 0);

			draw_quads(command_buffer);

			data.draw_calls++;
		}

		if (data.line_indices_submitted > 0) {
			const auto vertex_count = data.line_vertices_submitted;

			const auto size = vertex_count * sizeof(LineVertex);
			data.line_vertex_buffer->set_data(data.line_buffer.data(), size, 0);

			draw_lines(command_buffer);

			data.draw_calls++;
		}

		if (data.meshes_submitted > 0) {
			draw_meshes(command_buffer);
		}

		Renderer::end_render_pass(command_buffer);
	}

	void Renderer3D::draw_quads(const CommandBuffer& command_buffer)
	{
		const auto& vb = data.quad_vertex_buffer;
		const auto& ib = data.quad_index_buffer;
		const auto& descriptor = data.descriptor_sets[Renderer::current_frame()];
		const auto& pipeline = data.pipelines["quad"sv];
		const auto index_count = data.quad_indices_submitted;

		vkCmdBindPipeline(command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

		const auto& pc = data.push_constant;
		vkCmdPushConstants(command_buffer.get_buffer(), pipeline->get_vulkan_pipeline_layout(),
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PC), &pc);

		const std::array<VkBuffer, 1> vbs { vb->get_vulkan_buffer() };
		constexpr VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(command_buffer.get_buffer(), 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(command_buffer.get_buffer(), ib->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

		if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}

		const auto count = index_count;
		vkCmdDrawIndexed(command_buffer.get_buffer(), count, 1, 0, 0, 0);
	}

	void Renderer3D::draw_lines(const CommandBuffer& command_buffer)
	{
		const auto& vb = data.line_vertex_buffer;
		const auto& ib = data.line_index_buffer;
		const auto& descriptor = data.descriptor_sets[Renderer::current_frame()];
		const auto& pipeline = data.pipelines["line"sv];
		const auto index_count = data.line_indices_submitted;

		vkCmdBindPipeline(command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline());

		const std::array<VkBuffer, 1> vbs { vb->get_vulkan_buffer() };
		constexpr VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(command_buffer.get_buffer(), 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(command_buffer.get_buffer(), ib->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

		if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}

		const auto line_width = pipeline->get_specification().line_width;
		vkCmdSetLineWidth(command_buffer.get_buffer(), line_width);

		const auto count = index_count;
		const auto instances = index_count / 2;
		vkCmdDrawIndexed(command_buffer.get_buffer(), count, instances, 0, 0, 0);
	}

	void Renderer3D::draw_meshes(const CommandBuffer& command_buffer)
	{
		const VkDescriptorSet& descriptor = data.descriptor_sets[Renderer::current_frame()];

		const Pipeline* initial_pipeline = nullptr;
		VkPipelineLayout initial_layout = nullptr;
		const Mesh* initial_mesh = nullptr;

		for (std::uint32_t i = 0; i < data.meshes_submitted; i++) {
			const auto& mesh = data.mesh[i];
			const auto& vb = mesh->get_vertex_buffer();
			const auto& ib = mesh->get_index_buffer();
			const auto& pipeline = data.mesh_pipeline_submit[i];
			const auto& mesh_transform = data.mesh_transform[i];
			const auto& mesh_colour = data.mesh_colour[i];

			data.push_constant.object_transform = mesh_transform;
			data.push_constant.object_colour = mesh_colour;
			const auto& pc = data.push_constant;
			vkCmdPushConstants(command_buffer.get_buffer(), pipeline->get_vulkan_pipeline_layout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PC), &pc);

			if (initial_layout != pipeline->get_vulkan_pipeline_layout()) {
				initial_layout = pipeline->get_vulkan_pipeline_layout();
				vkCmdBindDescriptorSets(command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, initial_layout, 0, 1, &descriptor, 0, nullptr);
			}

			if (initial_pipeline != pipeline) {
				initial_pipeline = pipeline;
				vkCmdBindPipeline(command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, initial_pipeline->get_vulkan_pipeline());
			}

			if (initial_mesh != mesh) {
				initial_mesh = mesh;
				std::array<VkBuffer, 1> vbs { *vb };
				VkDeviceSize offsets { 0 };
				vkCmdBindVertexBuffers(command_buffer.get_buffer(), 0, 1, vbs.data(), &offsets);

				vkCmdBindIndexBuffer(command_buffer.get_buffer(), *ib, 0, VK_INDEX_TYPE_UINT32);
			}

			vkCmdDrawIndexed(command_buffer.get_buffer(), static_cast<std::uint32_t>(mesh->get_index_count()), 1, 0, 0, 0);
			data.draw_calls++;
		}
	}

	void Renderer3D::update_uniform_buffers(const std::optional<glm::mat4>& model)
	{
		const auto image_index = Application::the().swapchain().frame();

		UBO ubo {
			.model = model.value_or(default_model),
			.view = camera->get_view_matrix(),
			.projection = camera->get_projection_matrix(),
			.view_projection = ubo.projection * ubo.view,
		};

		data.uniforms[image_index]->set_data(&ubo, sizeof(UBO), 0);
	}

	void Renderer3D::destroy()
	{
		const auto& device = GraphicsContext::the().device();

		for (auto itr = data.pipelines.begin(); itr != data.pipelines.end();) {
			(*itr).second->destroy();
			itr = data.pipelines.erase(itr);
		}
		data.pipelines.clear();

		data.framebuffer->destroy();

		data.quad_vertex_buffer->destroy();
		data.quad_index_buffer->destroy();
		data.line_vertex_buffer->destroy();
		data.line_index_buffer->destroy();

#ifdef ALABASTER_WINDOWS
		std::ranges::for_each(data.uniforms.begin(), data.uniforms.end(), [](auto& uni) { uni->destroy(); });
#else
		std::for_each(data.uniforms.begin(), data.uniforms.end(), [](auto& uni) { uni->destroy(); });
#endif

		vkDestroyDescriptorPool(device, data.descriptor_pool, nullptr);
		vkDestroyDescriptorSetLayout(device, data.descriptor_set_layout, nullptr);
	}

	const VkRenderPass& Renderer3D::get_render_pass() const { return data.framebuffer->get_renderpass(); }

	void Renderer3D::set_camera(const std::shared_ptr<Camera>& cam) { camera = cam; }

} // namespace Alabaster
