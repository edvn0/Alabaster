#pragma once

#include <glm/glm.hpp>
#include <initializer_list>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	struct PushConstantRange {
		VkShaderStageFlags flags;
		std::uint32_t size;

		PushConstantRange(VkShaderStageFlags flags, std::uint32_t size)
			: flags(flags)
			, size(size) {};
	};

	struct PushConstantRanges {
		explicit PushConstantRanges(const std::initializer_list<PushConstantRange>& in)
			: ranges(in)
		{
			std::uint32_t offset = 0;
			for (const auto& range : ranges) {
				VkPushConstantRange out {};
				out.offset = offset;
				out.size = range.size;
				out.stageFlags = range.flags;
				output_ranges.push_back(out);
				offset += range.size;
			}
		};

		const auto& get_ranges() const { return output_ranges; }

	private:
		std::vector<PushConstantRange> ranges;
		std::vector<VkPushConstantRange> output_ranges;
	};

} // namespace Alabaster