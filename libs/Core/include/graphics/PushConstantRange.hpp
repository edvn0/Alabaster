#pragma once

#include <glm/glm.hpp>
#include <initializer_list>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	enum class PushConstantKind { Vertex = 1 << 0, Fragment = 1 << 1, Both = 1 << 2 };

	constexpr PushConstantKind operator|(PushConstantKind a, PushConstantKind b)
	{
		return static_cast<PushConstantKind>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	};

	constexpr VkShaderStageFlags to_vulkan_flags(PushConstantKind kind);

	struct PushConstantRange {
		constexpr PushConstantRange(PushConstantKind flags, std::uint32_t size)
			: size(size)
			, flags(flags) {};
		constexpr PushConstantRange()
			: size(0)
			, flags(PushConstantKind::Both) {};

		std::uint32_t size;
		PushConstantKind flags;
		std::uint32_t offset { 0 };
	};

	struct PushConstantRanges {
		explicit PushConstantRanges(const std::initializer_list<PushConstantRange>& in);
		const auto& get_ranges() const { return output_ranges; }

		auto size() const { return ranges.size(); }

	private:
		std::vector<PushConstantRange> ranges;
		std::vector<VkPushConstantRange> output_ranges;
	};

} // namespace Alabaster
