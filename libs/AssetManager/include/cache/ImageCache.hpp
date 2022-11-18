#pragma once

#include "cache/BaseCache.hpp"
#include "graphics/Image.hpp"

#include <unordered_map>

namespace AssetManager {

	template <class T> class ImageCache : public BaseCache<ImageCache, Alabaster::Image> {
	public:
		ImageCache(std::unique_ptr<CacheCreateRead<T>> cache_crud = std::make_unique<DefaultImageCrud>())
			: cache_crud(std::move(cache_crud)) {};

		void load_from_directory(const std::filesystem::path& texture_path);

	public:
		void destroy_impl()
		{
			for (auto& [key, image] : images) {
				image.destroy();
			}
		}

		[[nodiscard]] std::optional<const Alabaster::Image*> get_from_cache_impl(const std::string& name)
		{
			if (images.contains(name)) {
				return { cache_crud->get(name, images) };
			}
			return {};
		}

		[[nodiscard]] bool add_to_cache_impl(const std::string& name, Alabaster::Image* input)
		{
			if (images.contains(name))
				return false;
			cache_crud->create(name, input, images);
			return true;
		};

	private:
		std::unordered_map<std::string, Alabaster::Image> images;
		std::filesystem::path texture_path;
		std::unique_ptr<CacheCreateRead<T>> cache_crud;
	};

} // namespace AssetManager