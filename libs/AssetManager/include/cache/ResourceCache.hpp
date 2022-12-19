#pragma once

#include "cache/ShaderCache.hpp"
#include "cache/TextureCache.hpp"
#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <filesystem>
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

	public:
		const Alabaster::Texture* texture(const std::string& name);
		const Alabaster::Shader* shader(const std::string& name);

	private:
		ResourceCache();

	private:
		TextureCache<Alabaster::Texture> texture_cache;
		ShaderCache<Alabaster::Shader> shader_cache;
	};

	inline auto& the() { return ResourceCache::the(); }

	template <typename T> struct get_asset {
		const T* operator()(const std::string& name)
		{
			(void)name;
			return nullptr;
		};
	};

	template <> struct get_asset<Alabaster::Texture> {
		const Alabaster::Texture* operator()(const std::string& name) { return the().texture(name); }
	};

	template <> struct get_asset<Alabaster::Shader> {
		const Alabaster::Shader* operator()(const std::string& name) { return the().shader(name); }
	};

	template <typename T> const T* asset(const auto& name)
	{
		try {
			return get_asset<T>()(name);
		} catch (const Alabaster::AlabasterException& e) {
			return nullptr;
		}
	}

} // namespace AssetManager
