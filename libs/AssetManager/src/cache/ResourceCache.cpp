//
// Created by Edwin Carlsson on 2022-11-15.
//
#include "cache/ResourceCache.hpp"

#include "core/exceptions/AlabasterException.hpp"
#include "utilities/FileInputOutput.hpp"

#include <filesystem>

namespace AssetManager {

	ResourceCache::ResourceCache()
	{
		shader_cache.load_from_directory(Alabaster::IO::shaders());
		texture_cache.load_from_directory(Alabaster::IO::textures(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
		texture_cache.load_from_directory(Alabaster::IO::fonts(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
		texture_cache.load_from_directory(Alabaster::IO::editor_resources(), { "*" });
	}

	void ResourceCache::initialise() { the(); }

	void ResourceCache::shutdown()
	{
		texture_cache.destroy();
		shader_cache.destroy();
	}

	ResourceCache& ResourceCache::the()
	{
		static ResourceCache cache;
		return cache;
	}

	const std::shared_ptr<Alabaster::Texture>& ResourceCache::texture(const std::string& name)
	{
		const auto& found = texture_cache.get_from_cache(name);
		if (found) {
			return found;
		}
		throw Alabaster::AlabasterException("Texture [{}] not found.", name);
	}

	const std::shared_ptr<Alabaster::Shader>& ResourceCache::shader(const std::string& name)
	{
		const auto& found = shader_cache.get_from_cache(name);
		if (found) {
			return found;
		}
		throw Alabaster::AlabasterException("Shader [{}] not found.", name);
	}

} // namespace AssetManager
