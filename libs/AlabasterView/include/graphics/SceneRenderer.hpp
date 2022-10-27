#pragma once

#include "UniformBuffer.hpp"

#include <glm/glm.hpp>
#include <memory>

typedef struct VkPipeline_T* VkPipeline;
typedef struct VkDeviceMemory_T* VkDeviceMemory;

namespace Alabaster {

	class Mesh;
	class Camera;
	class Pipeline;

	struct UBO {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 view_projection;
	};

	class SceneRenderer {
	public:
		SceneRenderer() = default;

		void init();
		void update_uniform_buffers(const glm::mat4& view, const glm::mat4& projection);

	public:
		void basic_mesh(const std::unique_ptr<Mesh>& basic_mesh, const std::unique_ptr<Camera>& camera, const std::unique_ptr<Pipeline>& pipeline);

	private:
		void create_descriptor_set_layout();
		void create_descriptor_pool();
		void create_descriptor_sets();

		std::vector<UniformBuffer> ubo_memory;
		std::vector<VkBuffer> uniform_buffers;
		std::vector<VkDeviceMemory> uniform_buffers_memory;

		std::vector<VkDescriptorSet> descriptor_sets;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;
	};
} // namespace Alabaster