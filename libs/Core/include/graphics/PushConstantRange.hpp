#pragma once

#include <glm/glm.hpp>
#include <initializer_list>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	enum class PushConstantKind { Vertex = 1 << 0, Fragment = 1 << 1, Both = 1 << 2 };

	VkShaderStageFlags to_vulkan_flags(PushConstantKind kind);

	struct PushConstantRange {
		PushConstantRange(PushConstantKind flags, std::uint32_t size);
		PushConstantRange()
			: size(0)
			, flags(PushConstantKind::Both) {};

		std::uint32_t size;
		PushConstantKind flags;
		std::uint32_t offset { 0 };
	};

	struct PushConstantRanges {
		explicit PushConstantRanges(const std::initializer_list<PushConstantRange>& in);
		const auto& get_ranges() const { return output_ranges; }

		const auto size() const { return ranges.size(); }

	private:
		std::vector<PushConstantRange> ranges;
		std::vector<VkPushConstantRange> output_ranges;
	};

} // namespace Alabaster
