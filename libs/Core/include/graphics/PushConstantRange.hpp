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
			: ranges(in) {};

	public:
		VkPushConstantRange get_push_constant_range() const
		{
			VkPushConstantRange out {};
			std::uint32_t current_size { 0 };
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

} // namespace Alabaster