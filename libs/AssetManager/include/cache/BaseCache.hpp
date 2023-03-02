#pragma once

#include "graphics/Image.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"
#include "utilities/StringHash.hpp"

#include <optional>
#include <string>

namespace AssetManager {

	template <typename T>
	static constexpr auto remove_extension = [](const T& path, std::uint32_t count = 2) {
		if constexpr (std::is_same_v<std::string, T>) {
			auto converted_path = std::filesystem::path { path };
			if (count != 2) {
				auto out = converted_path.filename();
				for (std::uint32_t i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return converted_path.filename().replace_extension().replace_extension().string();
		} else {
			auto out_converted = std::filesystem::path { path.filename() };
			if (count != 2) {
				auto out = out_converted;
				for (std::uint32_t i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return out_converted.replace_extension().replace_extension().string();
		}
	};

	template<typename T>
	using StringMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;

	template<typename T> class BaseCache {
	public:
		virtual ~BaseCache() = default;

		[[nodiscard]] virtual std::optional<const T*> get_from_cache(const std::string& item_name) = 0;
		[[nodiscard]] virtual bool add_to_cache(const std::string& name, T* input) = 0;
		virtual void destroy() = 0;
	};

} // namespace AssetManager
