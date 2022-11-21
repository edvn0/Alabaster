#pragma once

#include "cache/ImageCache.hpp"
#include "cache/ShaderCache.hpp"
#include "graphics/Image.hpp"

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
		const Alabaster::Image& texture(const std::string& name);
		const Alabaster::Shader& shader(const std::string& name);

	private:
		ResourceCache();

	private:
		ImageCache<Alabaster::Image> image_cache;
		ShaderCache<Alabaster::Shader> shader_cache;
	};

	static inline auto& the() { return ResourceCache::the(); }

} // namespace AssetManager