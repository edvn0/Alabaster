#pragma once

#include <glm/glm.hpp>
#include <initializer_list>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	enum class PushConstantKind { Vertex = 1 << 0, Fragment = 1 << 1, Both = 1 << 2 };

	VkShaderStageFlags to_vulkan_flags(PushConstantKind kind);

	struct PushConstantRange {
		PushConstantKind flags;
		std::uint32_t size;
		PushConstantRange(PushConstantKind flags, std::uint32_t size);
	};

	struct PushConstantRanges {
		explicit PushConstantRanges(const std::initializer_list<PushConstantRange>& in);
		const auto& get_ranges() const { return output_ranges; }

	private:
		std::vector<PushConstantRange> ranges;
		std::vector<VkPushConstantRange> output_ranges;
	};

} // namespace Alabaster
