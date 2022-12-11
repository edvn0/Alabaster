#pragma once

#include "cache/BaseCache.hpp"
#include "graphics/Texture.hpp"

#include <unordered_map>
#include <unordered_set>

namespace AssetManager {

	template <class T> class TextureCache : public BaseCache<TextureCache, Alabaster::Texture> {
	public:
		TextureCache(std::unique_ptr<CacheCreateRead<T>> cache_crud = std::make_unique<DefaultTextureCrud>())
			: cache_crud(std::move(cache_crud)) {};

		void load_from_directory(
			const std::filesystem::path& texture_path, std::unordered_set<std::string> include_extensions = { ".tga", ".png", ".jpeg", ".jpg" });

	public:
		void destroy_impl()
		{
			for (auto& [key, value] : images) {
				value.destroy();
			}
		}

		[[nodiscard]] std::optional<const Alabaster::Texture*> get_from_cache_impl(const std::string& name)
		{
			if (images.contains(name)) {
				return { cache_crud->get(name, images) };
			}
			return {};
		}

		[[nodiscard]] bool add_to_cache_impl(const std::string& name, Alabaster::Texture* input)
		{
			if (images.contains(name))
				return false;
			try {
				cache_crud->create(name, input, images);
			} catch (const std::exception& exc) {
				throw Alabaster::AlabasterException(exc.what());
			}
			return true;
		};

	private:
		std::unordered_map<std::string, Alabaster::Texture> images;
		std::filesystem::path texture_path;
		std::unique_ptr<CacheCreateRead<T>> cache_crud;

		friend BaseCache;
	};

} // namespace AssetManager
