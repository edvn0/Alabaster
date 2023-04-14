#pragma once

#include "cache/BaseCache.hpp"
#include "compiler/ShaderCompiler.hpp"
#include "graphics/Shader.hpp"

#include <unordered_map>

namespace AssetManager {

	class ShaderCache {
	public:
		ShaderCache() = default;
		~ShaderCache() = default;

		void load_from_directory(const std::filesystem::path& shader_directory_path);

		void destroy()
		{
			for (auto& [key, shader] : shaders) {
				shader->destroy();
			}
		}

		[[nodiscard]] const std::shared_ptr<Alabaster::Shader>& get_from_cache(const std::string& name)
		{
			if (shaders.contains(name)) {
				return shaders.at(name);
			}

			throw Alabaster::AlabasterException("Could not find shader");
		}

		template <typename... Args> [[nodiscard]] bool add_to_cache(const std::string& name, Args&&... args)
		{
			if (shaders.contains(name)) {
				return false;
			}
			shaders.try_emplace(name, std::forward<Args>(args)...);
			return true;
		}

	private:
		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> extract_into_pairs_of_shaders(
			const std::vector<std::string>& sorted_shaders_in_directory) const;

		std::unordered_map<std::string, std::shared_ptr<Alabaster::Shader>> shaders;
	};

} // namespace AssetManager
