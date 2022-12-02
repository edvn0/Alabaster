#pragma once

#include "graphics/Image.hpp"
#include "graphics/UniformBuffer.hpp"
#include "vulkan/vulkan_core.h"

#include <array>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	// static constexpr auto default_model =glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	class Mesh;
	class Pipeline;
	class VertexBuffer;
	class IndexBuffer;
	class CommandBuffer;
	class Camera;

	struct QuadVertex {
		glm::vec4 position;
		glm::vec4 colour;
		glm::vec3 normals;
		glm::vec2 uvs;
	};

	struct LineVertex {
		glm::vec4 position;
		glm::vec4 colour;
	};

	struct UBO {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 view_projection;
	};

	struct PC {
		glm::vec4 light_position;
		glm::vec4 light_colour;
		glm::vec4 light_ambience { 0.1f };
		glm::vec4 object_colour;
		glm::mat4 object_transform;
	};

	struct RendererData {
		static constexpr std::uint32_t max_vertices = 20000;
		static constexpr std::uint32_t max_indices = 6 * max_vertices;
		std::uint32_t draw_calls { 0 };

		std::uint32_t quad_indices_submitted { 0 };
		std::uint32_t quad_vertices_submitted { 0 };
		std::shared_ptr<Pipeline> quad_pipeline;
		std::array<QuadVertex, max_vertices> quad_buffer;
		std::unique_ptr<VertexBuffer> quad_vertex_buffer;
		std::unique_ptr<IndexBuffer> quad_index_buffer;

		std::uint32_t line_indices_submitted { 0 };
		std::uint32_t line_vertices_submitted { 0 };
		std::shared_ptr<Pipeline> line_pipeline;
		std::array<LineVertex, max_vertices> line_buffer;
		std::unique_ptr<VertexBuffer> line_vertex_buffer;
		std::unique_ptr<IndexBuffer> line_index_buffer;

		std::vector<std::unique_ptr<UniformBuffer>> uniforms;

		std::vector<VkDescriptorSet> descriptor_sets;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;
		VkRenderPass render_pass;

		std::uint32_t meshes_submitted { 0 };
		std::array<Mesh*, 200> mesh;
		std::array<glm::mat4, 200> mesh_transform {};
		std::array<glm::vec4, 200> mesh_colour;
		std::array<Pipeline*, 200> mesh_pipeline_submit;
		std::shared_ptr<Pipeline> mesh_pipeline;

		std::shared_ptr<Mesh> sphere_model;
		PC push_constant;
	};

	class Renderer3D {
	public:
		explicit Renderer3D(Camera& camera) noexcept;

		void begin_scene();

		void quad(const glm::vec3& pos = { 0, 0, 0 }, const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 },
			float rotation_degrees = 0.0f);
		void quad(glm::mat4 transform, const glm::vec4& colour);

		void mesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Pipeline>& pipeline = nullptr, const glm::vec3& pos = { 0, 0, 0 },
			const glm::mat4& rotation_matrix = glm::mat4 { 1.0f }, const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 });
		void mesh(const std::shared_ptr<Mesh>& mesh, const glm::vec3& pos = { 0, 0, 0 }, const glm::vec4& colour = { 1, 1, 1, 1 },
			const glm::vec3& scale = { 1, 1, 1 });
		void mesh(const std::shared_ptr<Mesh>& mesh, glm::mat4 transform, const std::shared_ptr<Pipeline>& pipeline = nullptr,
			const glm::vec4& colour = { 1, 1, 1, 1 });
		void mesh(const std::shared_ptr<Mesh>& mesh, glm::mat4 transform, const glm::vec4& colour = { 1, 1, 1, 1 });

		void line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		void line(float size, const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		void text(std::string text, glm::vec3 position, float font_size = 11.0f);

		void end_scene(const std::unique_ptr<CommandBuffer>& command_buffer, VkRenderPass target = nullptr);

		void set_light_data(const glm::vec4& light_position, const glm::vec4& colour, float ambience = 1.0f);

	public:
		void destroy();
		void reset_stats();

	public:
		void set_camera(Camera& cam);

	public:
		const VkRenderPass& get_render_pass() const;

	private:
		void draw_quads(const std::unique_ptr<CommandBuffer>& command_buffer);
		void draw_lines(const std::unique_ptr<CommandBuffer>& command_buffer);
		void draw_meshes(const std::unique_ptr<CommandBuffer>& command_buffer);

	private:
		void flush();
		void update_uniform_buffers(const std::optional<glm::mat4>& model = {});
		void create_descriptor_set_layout();
		void create_descriptor_pool();
		void create_descriptor_sets();
		void create_renderpass();

	private:
		Camera& camera;
		RendererData data;
	};

} // namespace Alabaster
