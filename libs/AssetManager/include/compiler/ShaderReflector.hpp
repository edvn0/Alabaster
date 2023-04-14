#pragma once

#include "graphics/PushConstantRange.hpp"

#include <memory>
#include <spirv-cross/spirv_glsl.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace AssetManager {

	struct UniformBufferDefinition {
		std::string_view name;
		std::uint32_t size;
		std::uint32_t member_count;
		std::uint32_t binding;
		std::uint32_t descriptor_set;
	};

	struct ShaderReflectionData {
		std::vector<Alabaster::PushConstantRange> push_constant_ranges;
		std::vector<UniformBufferDefinition> uniform_buffers;
	};

	class ShaderReflector {
	public:
		explicit ShaderReflector(std::string shader_file_name, const std::vector<std::uint32_t>& spirv);

	private:
		std::unique_ptr<spirv_cross::CompilerGLSL> reflector;
		ShaderReflectionData reflection_data;
	};

} // namespace AssetManager
