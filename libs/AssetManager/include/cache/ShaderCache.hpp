#pragma once

#include "cache/BaseCache.hpp"
#include "compiler/ShaderCompiler.hpp"
#include "graphics/Shader.hpp"

#include <unordered_map>

namespace AssetManager {

	class ShaderCache : public BaseCache<Alabaster::Shader> {
	public:
		void load_from_directory(const std::filesystem::path& shader_directory_path);

		void destroy() override
		{
			for (auto& [key, shader] : shaders) {
				shader.destroy();
			}
		}

		[[nodiscard]] std::optional<const Alabaster::Shader*> get_from_cache(const std::string& name) override
		{
			if (shaders.contains(name)) {
				return { &shaders.at(name) };
			}
			return {};
		}

		[[nodiscard]] bool add_to_cache(const std::string& name, Alabaster::Shader* input) override
		{
			if (shaders.contains(name)) {
				return false;
			}
			shaders.try_emplace(name, *input);
			return true;
		};

	private:
		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> extract_into_pairs_of_shaders(
			const std::vector<std::string>& sorted_shaders_in_directory);

		StringMap<Alabaster::Shader> shaders;
	};

} // namespace AssetManager
