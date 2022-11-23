#pragma once

#include "graphics/PushConstantRange.hpp"

#include <memory>
#include <spirv-cross/spirv_glsl.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace AssetManager {

	struct ShaderReflectionData {
		std::vector<Alabaster::PushConstantRange> push_constant_ranges;
	};

	class ShaderReflector {
	public:
		explicit ShaderReflector(std::string shader_file_name, const std::vector<std::uint32_t>& spirv);

		const std::vector<VkPushConstantRange>& get_push_constant_range() const;

	private:
		std::unique_ptr<spirv_cross::CompilerGLSL> reflector;
		ShaderReflectionData reflection_data;
	};

} // namespace AssetManager
