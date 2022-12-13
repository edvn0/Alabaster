#include "am_pch.hpp"

#include "cache/TextureCache.hpp"

#include "graphics/CommandBuffer.hpp"
#include "graphics/Image.hpp"
#include "utilities/FileSystem.hpp"
#include "utilities/ThreadPool.hpp"

namespace AssetManager {

	template <class T>
	void TextureCache<T>::load_from_directory(const std::filesystem::path& directory, std::unordered_set<std::string> include_extensions)
	{
		using namespace Alabaster;
		auto sorted_images_in_directory = FS::in_directory<std::string, false>(directory, include_extensions, true);

		ImmediateCommandBuffer buffer { "Texture Cache" };
		ThreadPool pool { 8 };

		// Load all images and obtain pixels from image data (png, jpg etc)
		for (const auto& entry : sorted_images_in_directory) {
			const auto& image_name = remove_extension<std::filesystem::path>(entry);
			Alabaster::TextureProperties spec;
			spec.generate_mips = false;
			spec.debug_name = image_name;
			images.try_emplace(image_name, std::filesystem::path { entry }, spec); // Push into image/texture cache
		}
		pool.stop();
	}

	template class TextureCache<Alabaster::Texture>;

} // namespace AssetManager
