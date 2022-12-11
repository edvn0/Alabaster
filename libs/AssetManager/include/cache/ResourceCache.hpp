#pragma once

#include "cache/ShaderCache.hpp"
#include "cache/TextureCache.hpp"
#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <filesystem>

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
		const Alabaster::Texture& texture(const std::string& name);
		const Alabaster::Shader& shader(const std::string& name);

	private:
		ResourceCache();

	private:
		TextureCache<Alabaster::Texture> image_cache;
		ShaderCache<Alabaster::Shader> shader_cache;
	};

	inline auto& the() { return ResourceCache::the(); }

} // namespace AssetManager
