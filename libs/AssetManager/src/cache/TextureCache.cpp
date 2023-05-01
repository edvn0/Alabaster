#include "am_pch.hpp"

#include "cache/TextureCache.hpp"

#include "core/Utilities.hpp"
#include "filesystem/FileSystem.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/Image.hpp"
#include "utilities/ThreadPool.hpp"

namespace AssetManager {

	static constexpr auto num_batches = 4;

	void TextureCache::load_from_directory(
		const std::filesystem::path& directory, const std::unordered_set<std::string, StringHash, std::equal_to<>>& include_extensions)
	{
		using namespace Alabaster;
		auto sorted_images_in_directory
			= FileSystem::in_directory<std::filesystem::path, StringHash, std::equal_to<>>(directory, include_extensions, true);

		const auto batches = Utilities::split_into(sorted_images_in_directory, num_batches);
		for (const auto& batch : batches) {
			for (const auto& entry : batch) {
				const auto& image_name = entry.filename().string();
				Alabaster::TextureProperties spec;
				spec.generate_mips = false;
				spec.debug_name = image_name;

				textures.try_emplace(image_name, Texture::from_full_path(entry));
			}
		}
	}

} // namespace AssetManager
