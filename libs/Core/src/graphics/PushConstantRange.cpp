#include "av_pch.hpp"

#include "graphics/PushConstantRange.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"

#include <vulkan/vulkan.h>

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
		default:
			throw AlabasterException("PushConstantKind is never here.");
		}
	}

	PushConstantRanges::PushConstantRanges(const std::initializer_list<PushConstantRange>& in)
		: ranges(in)
	{
	}
} // namespace Alabaster
