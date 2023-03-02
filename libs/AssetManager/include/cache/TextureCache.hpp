#pragma once

#include "cache/BaseCache.hpp"
#include "graphics/Texture.hpp"

#include <unordered_map>
#include <unordered_set>

namespace AssetManager {

	class TextureCache : public BaseCache<Alabaster::Texture> {
	public:
		TextureCache() = default;
		~TextureCache() override = default;

		void load_from_directory(const std::filesystem::path& texture_path,
			const std::unordered_set<std::string, StringHash, std::equal_to<>>& include_extensions = { ".tga", ".png", ".jpeg", ".jpg" });

		void destroy() override
		{
			for (auto it = textures.begin(); it != textures.end();) {
				it->second.destroy();
				it = textures.erase(it);
			}
			textures.clear();
		}

		[[nodiscard]] std::optional<const Alabaster::Texture*> get_from_cache(const std::string& name) override
		{
			if (textures.contains(name)) {
				return { &textures.at(name) };
			}
			return {};
		}

		[[nodiscard]] bool add_to_cache(const std::string& name, Alabaster::Texture* input) override
		{
			if (textures.contains(name))
				return false;
			try {
				textures.try_emplace(name, *input);
			} catch (const std::exception& exc) {
				throw Alabaster::AlabasterException(exc.what());
			}
			return true;
		};

	private:
		StringMap<Alabaster::Texture> textures;
		std::filesystem::path texture_path;
	};

} // namespace AssetManager
