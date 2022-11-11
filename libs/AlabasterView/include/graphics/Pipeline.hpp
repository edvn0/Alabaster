#pragma once

#include "graphics/Shader.hpp"
#include "graphics/VertexBufferLayout.hpp"
#include "vulkan/vulkan_core.h"

#include <initializer_list>
#include <string>
#include <vulkan/vulkan.h>

namespace Alabaster {

	struct PushConstantRange {
		VkShaderStageFlags flags;
		uint32_t size;

		PushConstantRange(VkShaderStageFlags flags, uint32_t size)
			: flags(flags)
			, size(size) {};
	};

	struct PushConstantRanges {
		explicit PushConstantRanges(const std::initializer_list<PushConstantRange>& in)
			: ranges(in) {};

	public:
		VkPushConstantRange get_push_constant_range() const
		{
			VkPushConstantRange out {};
			uint32_t current_size { 0 };
			auto f = VK_SHADER_STAGE_VERTEX_BIT;
			VkShaderStageFlags flags {};
			for (const auto& range : ranges) {
				current_size += range.size;
				flags |= range.flags;
			}

			out.size = current_size;
			out.stageFlags = flags;
			out.offset = 0;
			return out;
		}

	private:
		std::vector<PushConstantRange> ranges;
	};

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
		std::optional<PushConstantRanges> ranges { std::nullopt };
		float line_width { 1.0f };
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

		bool operator!=(const Pipeline& other) const { return pipeline != other.pipeline; }

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
