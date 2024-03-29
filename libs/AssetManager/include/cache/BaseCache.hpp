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

} // namespace AssetManager
