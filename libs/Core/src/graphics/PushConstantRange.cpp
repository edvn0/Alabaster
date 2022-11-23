#include "av_pch.hpp"

#include "graphics/PushConstantRange.hpp"

namespace Alabaster {

	VkShaderStageFlags to_vulkan_flags(PushConstantKind kind)
	{
		switch (kind) {
		case PushConstantKind::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case PushConstantKind::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case PushConstantKind::Both:
			return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		}
	}

	PushConstantRanges::PushConstantRanges(const std::initializer_list<PushConstantRange>& in)
		: ranges(in)
	{
		std::uint32_t offset = 0;
		for (const auto& range : ranges) {
			VkPushConstantRange out {};
			out.offset = offset;
			out.size = range.size;
			out.stageFlags = to_vulkan_flags(range.flags);
			output_ranges.push_back(out);
			offset += range.size;
		}
	};

	PushConstantRange::PushConstantRange(PushConstantKind flags, std::uint32_t size)
		: flags(flags)
		, size(size) {};

} // namespace Alabaster
