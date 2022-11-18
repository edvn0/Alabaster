#pragma once

#include "graphics/Shader.hpp"

#include <filesystem>
#include <future>
#include <string>
#include <unordered_map>
#include <vector>

namespace AssetManager {

	class ShaderCompiler {
	public:
		ShaderCompiler() = default;
		explicit ShaderCompiler(const std::filesystem::path& shader_directory);

		void destroy();

		const Alabaster::Shader& get_by_name(const std::string& basic_string);

	private:
		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> extract_into_pairs_of_shaders(
			const std::vector<std::string>& sorted_shaders_in_directory);

		std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> compile(
			const std::string& name, const std::filesystem::path& vertex, const std::filesystem::path& fragment);

	private:
		std::unordered_map<std::string, Alabaster::Shader> shaders;
	};
} // namespace AssetManager
