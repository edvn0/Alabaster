#pragma once

#include "graphics/Pipeline.hpp"

namespace std {
	template <> struct hash<Alabaster::Pipeline> {
		std::std::size_t operator()(Alabaster::Pipeline pipeline) const { return std::hash<VkPipeline> {}(pipeline.get_vulkan_pipeline()); }
	};
} // namespace std

namespace Alabaster {

	class Material final {
	public:
		Material(const Pipeline& pipeline)
			: pipeline(pipeline)
		{
		}

		auto get_pipeline() const { return pipeline.get_vulkan_pipeline(); };
		auto get_layout() const { return pipeline.get_vulkan_pipeline_layout(); };
		std::size_t identifier() const { return std::hash<VkPipeline>(pipeline); }

	private:
		Pipeline pipeline;
	};

} // namespace Alabaster
