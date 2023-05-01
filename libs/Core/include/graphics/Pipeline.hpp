#pragma once

#include "graphics/PushConstantRange.hpp"
#include "graphics/Shader.hpp"
#include "graphics/VertexBufferLayout.hpp"

#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using VkPipelineLayout = struct VkPipelineLayout_T*;
using VkPipeline = struct VkPipeline_T*;
using VkRenderPass = struct VkRenderPass_T*;
using VkPipelineCache = struct VkPipelineCache_T*;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Alabaster {

	enum class Topology : std::uint8_t {
		PointList = 0,
		LineList = 1,
		LineStrip = 2,
		TriangleList = 3,
		TriangleStrip = 4,
		TriangleFan = 5,
		AdjacencyLineList = 6,
		AdjacencyLineStrip = 7,
		AdjacencyTriangleList = 8,
		AdjacencyTriangleStrip = 9,
		PatchList = 10,
	};

	struct PipelineSpecification {
		std::shared_ptr<Shader> shader;
		bool shader_owned_by_pipeline { false };
		std::string debug_name;
		VkRenderPass render_pass { nullptr };
		bool wireframe { false };
		bool backface_culling { false };
		Topology topology = Topology::TriangleList;
		bool depth_test { true };
		bool depth_write { true };
		VertexBufferLayout vertex_layout {};
		VertexBufferLayout instance_layout {};
		std::optional<PushConstantRanges> ranges { std::nullopt };
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts {};
		float line_width { 1.0f };
	};

	class Pipeline {
	public:
		~Pipeline();

		void invalidate();

		PipelineSpecification& get_specification();
		const PipelineSpecification& get_specification() const;

		VkPipelineLayout get_vulkan_pipeline_layout() const;
		VkPipeline get_vulkan_pipeline() const;

		bool operator!=(const Pipeline& other) const;
		bool operator()(const Pipeline* other) const;

		inline static std::shared_ptr<Pipeline> create(PipelineSpecification spec)
		{
			auto pipeline = std::shared_ptr<Pipeline>(new Pipeline { spec });
			pipeline->invalidate();
			return pipeline;
		}

	private:
		PipelineSpecification spec;
		VkPipelineLayout pipeline_layout {};
		VkPipeline pipeline {};
		VkPipelineCache pipeline_cache = nullptr;
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts;

		explicit Pipeline(PipelineSpecification pipe_spec)
			: spec(std::move(pipe_spec)) {};
	};

} // namespace Alabaster
