#pragma once

#include <array>
#include <filesystem>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class Shader {
	public:
		/// @brief Create a shader.
		///     This looks for <path_and_filename>.vert.spv and <path_and_filename>.frag.spv
		///     or <path_and_filename>-vert.spv and <path_and_filename>-frag.spv
		/// @param path_and_filename
		explicit Shader(const std::filesystem::path& path_and_filename);
		Shader(const std::string& vertex_path, const std::string& fragment_path);
		Shader(const std::string& path_or_name, std::vector<std::uint32_t> vertex_shader_spirv, std::vector<std::uint32_t> fragment_shader_spirv);

		const std::array<VkPipelineShaderStageCreateInfo, 2>& stages() const { return shader_stages; };

		void destroy();

		const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts() const { return layouts; }

	private:
		void create_layout();

	private:
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {};
		std::filesystem::path shader_path;
		std::vector<VkDescriptorSetLayout> layouts;
	};

} // namespace Alabaster
