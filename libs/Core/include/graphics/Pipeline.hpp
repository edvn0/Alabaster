#pragma once

#include "graphics/PushConstantRange.hpp"
#include "graphics/Shader.hpp"
#include "graphics/VertexBufferLayout.hpp"
#include "vulkan/vulkan_core.h"

#include <initializer_list>
#include <string>
#include <vulkan/vulkan.h>

namespace Alabaster {
	struct PipelineSpecification {
		Shader shader;
		bool shader_owned_by_pipeline { false };
		std::string debug_name;
		VkRenderPass render_pass { nullptr };
		bool wireframe { false };
		bool backface_culling { false };
		VkPrimitiveTopology topology;
		bool depth_test { true };
		bool depth_write { true };
		VertexBufferLayout vertex_layout;
		VertexBufferLayout instance_layout;
		std::optional<PushConstantRanges> ranges { std::nullopt };
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts {};
		float line_width { 1.0f };
	};

	class Pipeline {
	public:
		explicit Pipeline(PipelineSpecification spec)
			: spec(std::move(spec)) {};
		~Pipeline()
		{
			if (!destroyed)
				destroy();
		};

		void destroy();

		void invalidate();

		PipelineSpecification& get_specification() { return spec; }
		const PipelineSpecification& get_specification() const { return spec; }

		VkPipelineLayout get_vulkan_pipeline_layout() const { return pipeline_layout; }
		VkPipeline get_vulkan_pipeline() const { return pipeline; }

		bool operator!=(const Pipeline& other) const { return pipeline != other.pipeline; }
		bool operator()(const Pipeline* other) const { return spec.debug_name < other->spec.debug_name; }

	public:
		inline static std::shared_ptr<Pipeline> create(PipelineSpecification spec)
		{
			auto pipeline = std::make_shared<Pipeline>(spec);
			pipeline->invalidate();
			return pipeline;
		}

	private:
		bool destroyed { false };
		PipelineSpecification spec;
		VkPipelineLayout pipeline_layout {};
		VkPipeline pipeline {};
		VkPipelineCache pipeline_cache = nullptr;
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
		std::vector<VkPushConstantRange> push_constants_ranges;
	};

} // namespace Alabaster
