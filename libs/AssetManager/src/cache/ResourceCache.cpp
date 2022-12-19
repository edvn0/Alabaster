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
		texture_cache.load_from_directory(Alabaster::IO::textures(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
		texture_cache.load_from_directory(Alabaster::IO::fonts(), { ".png", ".tga", ".jpg", ".jpeg", ".bmp" });
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

	const Alabaster::Texture* ResourceCache::texture(const std::string& name)
	{
		const auto found = texture_cache.get_from_cache(name);
		if (!found) {
			throw Alabaster::AlabasterException("Texture not found.");
		}

		return found.value();
	}

	const Alabaster::Shader* ResourceCache::shader(const std::string& name)
	{
		const auto found = shader_cache.get_from_cache(name);
		if (!found) {
			try {
				const auto could_add = shader_cache.add_to_cache(name, new Alabaster::Shader { name });
				if (could_add) {
					const auto other_found = shader_cache.get_from_cache(name);
					return other_found.value();
				}
			} catch (const Alabaster::AlabasterException&) {
				throw Alabaster::AlabasterException(fmt::format(
					"Tried finding shader with name {} in both cache and in {}/shaders, but to no avail.", name, Alabaster::IO::shaders().string()));
			}
		}

		return found.value();
	}

} // namespace AssetManager
