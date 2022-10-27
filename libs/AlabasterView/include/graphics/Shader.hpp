#pragma once

#include <array>
#include <filesystem>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class Shader {
	public:
		/// @brief Create a shader.
		///     This looks for <path_and_filename>.vert.spv and <path_and_filename>.frag.spv
		///     or <path_and_filename>-vert.spv and <path_and_filename>-frag.spv
		/// @param path_and_filename
		explicit Shader(const std::filesystem::path& path_and_filename);

		inline std::array<VkPipelineShaderStageCreateInfo, 2> stages() const { return shader_stages; };

		void destroy();

		const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts() const { return layouts; }

	private:
		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {};
		std::filesystem::path shader_path;
		std::vector<VkDescriptorSetLayout> layouts;
	};

} // namespace Alabaster
