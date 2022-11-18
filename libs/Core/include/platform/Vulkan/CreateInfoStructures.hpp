#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vulkan/vulkan.h>

namespace Alabaster::Vulkan {
	enum class ShaderType : uint32_t {
		Vertex = 0x00000001,
		Geometry = 0x00000008,
		Fragment = 0x00000010,
		Compute = 0x00000020,
	};

	namespace Shader {
		const VkShaderModuleCreateInfo module(size_t size, const uint32_t* code);
		const VkPipelineShaderStageCreateInfo pipeline_stage(ShaderType which, VkShaderModule module, std::string_view entry_point = "main");
	} // namespace Shader
} // namespace Alabaster::Vulkan