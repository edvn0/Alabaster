#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <string_view>
#include <tuple>
#include <vector>

struct VkPipelineShaderStageCreateInfo;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Alabaster {

	class Shader {
	public:
		/// @brief Create a shader.
		///     This looks for <path_and_filename>.vert.spv and <path_and_filename>.frag.spv
		///     or <path_and_filename>-vert.spv and <path_and_filename>-frag.spv
		/// @param path_and_filename
		explicit Shader(std::tuple<std::string_view, std::string_view> vertex_fragment_paths);
		Shader(const std::string& path_or_name, std::vector<std::uint32_t> vertex_shader_spirv, std::vector<std::uint32_t> fragment_shader_spirv);

		template <typename Str = std::string_view>
		Shader(Str&& vertex, Str&& fragment)
			: Shader(std::make_tuple<Str, Str>(std::forward<Str>(vertex), std::forward<Str>(fragment)))
		{
		}
		Shader(std::string_view path_or_name, std::vector<std::uint32_t> vertex_shader_spirv, std::vector<std::uint32_t> fragment_shader_spirv)
			: Shader(std::string { path_or_name }, std::move(vertex_shader_spirv), std::move(fragment_shader_spirv)) {};

		~Shader();

		Shader(Shader&&);

		const std::array<VkPipelineShaderStageCreateInfo, 2> stages() const;

		void destroy();
		const auto& get_path() const { return shader_path; }
		const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts() const;

	private:
		void create_layout();

	private:
		std::unique_ptr<VkPipelineShaderStageCreateInfo> vertex_stage;
		std::unique_ptr<VkPipelineShaderStageCreateInfo> fragment_stage;
		std::filesystem::path shader_path;
		std::vector<VkDescriptorSetLayout> layouts;
	};

} // namespace Alabaster
