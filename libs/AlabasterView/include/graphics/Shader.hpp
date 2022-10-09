#pragma once

#include <filesystem>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class Shader {
	public:
		/// @brief Create a shader.This looks for <path_and_filename>.vert.spv and <path_and_filename>.frag.spv or <path_and_filename>-vert.spv and
		/// <path_and_filename>-frag.spv
		/// @param path_and_filename
		Shader(const std::filesystem::path& path_and_filename);

		VkShaderModule vertex() { return vertex_shader_module; }
		VkShaderModule fragment() { return fragment_shader_module; }

	private:
		VkShaderModule vertex_shader_module;
		VkShaderModule fragment_shader_module;
	};

} // namespace Alabaster
