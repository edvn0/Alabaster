#pragma once

#include "graphics/Shader.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace AlabasterShaderCompiler {

	class ShaderCompiler {
	public:
		explicit ShaderCompiler(const std::filesystem::path& shader_directory);
		~ShaderCompiler();

	private:
		std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> compile(
			const std::string& name, const std::filesystem::path& vertex, const std::filesystem::path& fragment);
		std::unordered_map<std::string, Alabaster::Shader> shaders;
	};
} // namespace AlabasterShaderCompiler
