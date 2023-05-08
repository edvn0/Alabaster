//
// Created by Edwin Carlsson on 2022-11-15.
//
#include "cache/ResourceCache.hpp"

#include "core/exceptions/AlabasterException.hpp"
#include "filesystem/FileSystem.hpp"

#include <filesystem>

namespace AssetManager {

	ResourceCache::ResourceCache()
	{
		shader_cache.load_from_directory(Alabaster::FileSystem::shaders());
		texture_cache.load_from_directory(Alabaster::FileSystem::textures(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
		texture_cache.load_from_directory(Alabaster::FileSystem::fonts(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
		texture_cache.load_from_directory(Alabaster::FileSystem::editor_resources(), { "*" });
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
		if (const auto& found = texture_cache.get_from_cache(name)) {
			return found;
		}

		try {
			if (texture_cache.add_to_cache(name, Alabaster::Texture::from_filename(name)))
				return texture_cache.get_from_cache(name);
		} catch (const Alabaster::AlabasterException& e) {
			throw Alabaster::AlabasterException("Exception: {}", e.what());
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
