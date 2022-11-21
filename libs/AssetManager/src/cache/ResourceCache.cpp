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
		image_cache.load_from_directory(Alabaster::IO::textures(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
		image_cache.load_from_directory(Alabaster::IO::fonts(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
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

	const Alabaster::Image& ResourceCache::texture(const std::string& name)
	{
		const auto found = image_cache.get_from_cache(name);
		if (!found) {
			throw Alabaster::AlabasterException("Texture not found.");
		}

		return *found.value();
	}

	const Alabaster::Shader& ResourceCache::shader(const std::string& name)
	{
		const auto found = shader_cache.get_from_cache(name);
		if (!found) {
			throw Alabaster::AlabasterException("Shader not found.");
		}

		return *found.value();
	}

} // namespace AssetManager
