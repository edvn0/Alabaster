#pragma once

#include "cache/ShaderCache.hpp"
#include "cache/TextureCache.hpp"

#include <filesystem>
#include <string_view>
#include <type_traits>

namespace AssetManager {

	class ResourceCache {
	public:
		ResourceCache(const ResourceCache&) = delete;
		void operator=(const ResourceCache&) = delete;
		ResourceCache(ResourceCache&&) = delete;

		void initialise();
		void shutdown();

		static ResourceCache& the();

		const std::shared_ptr<Alabaster::Texture>& texture(const std::string& name);
		const std::shared_ptr<Alabaster::Shader>& shader(const std::string& name);

	private:
		ResourceCache();

		TextureCache texture_cache;
		ShaderCache shader_cache;
	};

	inline auto& the() { return ResourceCache::the(); }

	template <typename T> struct get_asset {
		const std::shared_ptr<T>& operator()(const std::string& name) const = delete;
	};

	template <> struct get_asset<Alabaster::Texture> {
		const std::shared_ptr<Alabaster::Texture>& operator()(const std::string& name) const { return the().texture(name); }
	};

	template <> struct get_asset<Alabaster::Shader> {
		const std::shared_ptr<Alabaster::Shader>& operator()(const std::string& name) const { return the().shader(name); }
	};

	template <typename T, typename String> const std::shared_ptr<T>& asset(String&& name)
	{
		try {
			return get_asset<T>()(name);
		} catch (const Alabaster::AlabasterException& exc) {
			Alabaster::Log::info("Could not find asset with name {}. Exception: {}", name, exc.what());
			throw Alabaster::AlabasterException("{}", exc.what());
		}
	}

} // namespace AssetManager
