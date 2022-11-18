//
// Created by Edwin Carlsson on 2022-11-15.
//
#include "cache/ResourceCache.hpp"

#include "utilities/FileInputOutput.hpp"

#include <filesystem>

namespace AssetManager {

	ResourceCache::ResourceCache()
	{
		shader_cache.load_from_directory(Alabaster::IO::shaders());
		image_cache.load_from_directory(Alabaster::IO::textures());
	}

	void ResourceCache::initialise() { the(); }

	void ResourceCache::shutdown()
	{
		image_cache.destroy();
		shader_cache.destroy();
	}

	ResourceCache& ResourceCache::the()
	{
		static ResourceCache cache;
		return cache;
	}

	std::optional<const Alabaster::Image*> ResourceCache::texture(const std::string& name) { return image_cache.get_from_cache(name); }
	std::optional<const Alabaster::Shader*> ResourceCache::shader(const std::string& name) { return shader_cache.get_from_cache(name); }

} // namespace AssetManager
