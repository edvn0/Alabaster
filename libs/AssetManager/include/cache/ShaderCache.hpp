#pragma once

#include "cache/BaseCache.hpp"
#include "compiler/ShaderCompiler.hpp"
#include "graphics/Shader.hpp"

#include <unordered_map>

namespace AssetManager {

	template <class T> class ShaderCache : public BaseCache<ShaderCache, Alabaster::Shader> {
	public:
		ShaderCache(std::unique_ptr<CacheCreateRead<T>> cache_crud = std::make_unique<DefaultShaderCrud>())
			: cache_crud(std::move(cache_crud)) {};

		void load_from_directory(const std::filesystem::path& shader_directory_path);

	public:
		void destroy_impl()
		{
			for (auto& [key, shader] : shaders) {
				shader.destroy();
			}
		}

		[[nodiscard]] virtual std::optional<const Alabaster::Shader*> get_from_cache_impl(const std::string& name)
		{
			if (shaders.contains(name))
				return cache_crud->get(name, shaders);
			return {};
		}

		[[nodiscard]] virtual bool add_to_cache_impl(const std::string& name, Alabaster::Shader* input)
		{
			if (shaders.contains(name)) {
				return false;
			}
			cache_crud->create(name, input, shaders);
			return true;
		};

	private:
		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> extract_into_pairs_of_shaders(
			const std::vector<std::string>& sorted_shaders_in_directory);

	private:
		std::unordered_map<std::string, Alabaster::Shader> shaders;
		std::unique_ptr<CacheCreateRead<T>> cache_crud;

		friend BaseCache;
	};

} // namespace AssetManager
