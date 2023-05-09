#pragma once

#include "graphics/Framebuffer.hpp"
#include "graphics/Image.hpp"
#include "graphics/UniformBuffer.hpp"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <optional>
#include <vector>

using VkRenderPass = struct VkRenderPass_T*;

namespace Alabaster {

	class Mesh;
	class Pipeline;
	class VertexBuffer;
	class IndexBuffer;
	class CommandBuffer;
	class Camera;
	class Texture;

	struct RendererData;

	struct PointLight {
		glm::vec4 position;
		glm::vec4 ambience;
	};

	class Renderer3D {
	public:
		explicit Renderer3D(Camera* camera) noexcept;
		~Renderer3D();

		void begin_scene(const glm::mat4& projection, const glm::mat4& view);
		void begin_scene();

		void quad(const glm::vec3& pos = { 0, 0, 0 }, const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 },
			float rotation_degrees = 0.0f, int texture_id = 0);
		void quad(const glm::mat4& transform, const glm::vec4& colour, int texture_id = 0);

		void mesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Pipeline>& pipeline = nullptr, const glm::vec3& pos = { 0, 0, 0 },
			const glm::mat4& rotation_matrix = glm::mat4 { 1.0f }, const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 });
		void mesh(const std::shared_ptr<Mesh>& mesh, const glm::vec3& pos = { 0, 0, 0 }, const glm::vec4& colour = { 1, 1, 1, 1 },
			const glm::vec3& scale = { 1, 1, 1 });
		void mesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, const std::shared_ptr<Pipeline>& pipeline = nullptr,
			const glm::vec4& colour = { 1, 1, 1, 1 });
		void mesh(const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform, const glm::vec4& colour = { 1, 1, 1, 1 });

		void line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		void line(float size, const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		void text(std::string text, glm::vec3 position, float font_size = 11.0f);

		void end_scene(const CommandBuffer& command_buffer);
		void end_scene(const CommandBuffer& command_buffer, const Framebuffer& target);

		void set_light_data(const glm::vec4& light_position, const glm::vec4& colour, float ambience = 1.0f);
		void set_light_data(const glm::vec3& light_position, const glm::vec4& colour, const glm::vec4& ambience);
		void submit_point_light_data(const PointLight& point_light);
		void commit_point_light_data();

		std::size_t default_push_constant_size() const;

		void reset_stats();

		void set_camera(const Camera& cam);

		const VkRenderPass& get_render_pass() const;

	private:
		void draw_quads(const CommandBuffer& command_buffer);
		void draw_lines(const CommandBuffer& command_buffer);
		void draw_meshes(const CommandBuffer& command_buffer);

		void flush();
		void update_uniform_buffers(const glm::mat4& projection, const glm::mat4& view);
		void update_uniform_buffers();
		void create_descriptor_set_layout();
		void create_descriptor_pool();
		void create_descriptor_sets();

		void invalidate_pipelines();

		Camera* camera;
		RendererData* data;
		bool scene_has_begun { false };
	};

} // namespace Alabaster
