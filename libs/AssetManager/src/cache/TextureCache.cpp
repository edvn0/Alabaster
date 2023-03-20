#include "am_pch.hpp"

#include "cache/TextureCache.hpp"

#include "core/Utilities.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/Image.hpp"
#include "utilities/FileSystem.hpp"
#include "utilities/ThreadPool.hpp"

namespace AssetManager {

	void TextureCache::load_from_directory(
		const std::filesystem::path& directory, const std::unordered_set<std::string, StringHash, std::equal_to<>>& include_extensions)
	{
		using namespace Alabaster;
		auto sorted_images_in_directory = FS::in_directory<std::filesystem::path, false>(directory, include_extensions, true);

		ImmediateCommandBuffer buffer { "Texture Cache" };
		ThreadPool pool { 8 };

		const auto batches = Utilities::split_into(sorted_images_in_directory, 2);

		std::mutex single_entry;
		// Load all images and obtain pixels from image data (png, jpg etc)
		for (const auto& batch : batches) {
			pool.push([&batch, &textures = textures, &single_entry](int) {
				for (const auto& entry : batch) {
					const auto& image_name = entry.filename().string();
					Alabaster::TextureProperties spec;
					spec.generate_mips = false;
					spec.debug_name = image_name;

					std::unique_lock lock { single_entry };
					auto texture = std::make_shared<Alabaster::Texture>(std::filesystem::path { entry }, spec);
					textures.insert(std::make_pair(image_name, texture));
				}
			});
		}
		pool.stop(true);
	}

} // namespace AssetManager
