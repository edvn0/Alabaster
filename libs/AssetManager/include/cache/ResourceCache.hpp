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

		const Alabaster::Texture* texture(const std::string& name);
		const Alabaster::Shader* shader(const std::string& name);

	private:
		ResourceCache();

		TextureCache texture_cache;
		ShaderCache shader_cache;
	};

	inline auto& the() { return ResourceCache::the(); }

	template <typename T> struct get_asset {
		const T* operator()(auto&& name) const;
	};

	template <> struct get_asset<Alabaster::Texture> {
		const Alabaster::Texture* operator()(auto&& name) const { return the().texture(name); }
	};

	template <> struct get_asset<Alabaster::Shader> {
		const Alabaster::Shader* operator()(auto&& name) const { return the().shader(name); }
	};

	template <typename T> const T* asset(const auto& name)
	{
		try {
			return get_asset<T>()(name);
		} catch (const Alabaster::AlabasterException& exc) {
			Alabaster::Log::info("Could not find asset with name {}. Exception: {}", name, exc.what());
			return nullptr;
		}
	}

} // namespace AssetManager
