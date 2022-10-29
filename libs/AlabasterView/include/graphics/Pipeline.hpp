#pragma once

#include "graphics/Shader.hpp"
#include "graphics/VertexBufferLayout.hpp"

#include <string>
#include <vulkan/vulkan.h>

namespace Alabaster {

	struct PipelineSpecification {
		Shader shader;
		std::string debug_name;
		VkRenderPass render_pass { nullptr };
		bool wireframe { false };
		bool backface_culling { true };
		VkPrimitiveTopology topology;
		bool depth_test { true };
		bool depth_write { true };
		VertexBufferLayout vertex_layout;
		VertexBufferLayout instance_layout;
	};

	class Pipeline {
	public:
		explicit Pipeline(PipelineSpecification spec)
			: spec(std::move(spec)) {};
		~Pipeline() = default;

		void destroy();

		void invalidate();

		PipelineSpecification& get_specification() { return spec; }
		const PipelineSpecification& get_specification() const { return spec; }

		VkPipelineLayout get_vulkan_pipeline_layout() const { return pipeline_layout; }
		VkPipeline get_vulkan_pipeline() const { return pipeline; }

	public:
		inline static std::unique_ptr<Pipeline> create(PipelineSpecification spec)
		{
			auto pipeline = std::make_unique<Pipeline>(spec);
			pipeline->invalidate();
			return pipeline;
		}

	private:
		PipelineSpecification spec;
		VkPipelineLayout pipeline_layout {};
		VkPipeline pipeline {};
		VkPipelineCache pipeline_cache = nullptr;
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
		std::vector<VkPushConstantRange> push_constants_ranges;
	};

} // namespace Alabaster
