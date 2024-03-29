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

		Alabaster::Shader compile(
			const std::string& name, const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path) const;

	private:
		std::tuple<std::vector<std::uint32_t>, std::vector<std::uint32_t>> compile_to_spirv(
			const std::string& name, const std::filesystem::path& vertex, const std::filesystem::path& fragment) const;
	};
} // namespace AssetManager
