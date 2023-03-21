#pragma once

#include "cache/BaseCache.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "graphics/Texture.hpp"

#include <unordered_map>
#include <unordered_set>

namespace AssetManager {

	class TextureCache {
	public:
		TextureCache() = default;
		~TextureCache() = default;

		void load_from_directory(const std::filesystem::path& texture_path,
			const std::unordered_set<std::string, StringHash, std::equal_to<>>& include_extensions = { ".tga", ".png", ".jpeg", ".jpg" });

		void destroy()
		{
			for (auto it = textures.begin(); it != textures.end();) {
				it->second->destroy();
				it = textures.erase(it);
			}
			textures.clear();
		}

		[[nodiscard]] const std::shared_ptr<Alabaster::Texture>& get_from_cache(const std::string& name)
		{
			if (textures.contains(name)) {
				return textures[name];
			}

			throw Alabaster::AlabasterException("Could not find texture.");
		}

		template <typename... Args> [[nodiscard]] bool add_to_cache(const std::string& name, Args&&... args)
		{
			if (textures.contains(name))
				return false;
			try {
				textures.try_emplace(name, std::forward<Args>(args)...);
			} catch (const std::exception& exc) {
				throw Alabaster::AlabasterException(exc.what());
			}
			return true;
		}

	private:
		std::unordered_map<std::string, std::shared_ptr<Alabaster::Texture>> textures;
		std::filesystem::path texture_path;
	};

} // namespace AssetManager
