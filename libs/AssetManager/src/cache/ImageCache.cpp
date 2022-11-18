#include "am_pch.hpp"

#include "cache/ImageCache.hpp"

#include "utilities/FileInputOutput.hpp"
#include "utilities/ThreadPool.hpp"

#include <debug_break.h>
#include <future>

namespace AssetManager {

	struct ImageAndName {
		std::string name;
		Alabaster::Image image;
	};

	template <typename T>
	static constexpr auto remove_extension = [](const T& path, uint32_t count = 2) {
		if constexpr (std::is_same_v<std::string, T>) {
			auto converted_path = std::filesystem::path { path };
			if (count != 2) {
				auto out = converted_path.filename();
				for (auto i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return converted_path.filename().replace_extension().replace_extension().string();
		} else {
			auto out_converted = std::filesystem::path { path.filename() };
			if (count != 2) {
				auto out = out_converted;
				for (auto i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return out_converted.replace_extension().replace_extension().string();
		}
	};

	template <class T> void ImageCache<T>::load_from_directory(const std::filesystem::path& shader_directory)
	{
		std::vector<std::string> images_in_directory
			= Alabaster::IO::in_directory<std::string>(shader_directory, { ".tga", ".png", ".jpeg", ".jpg" });
		std::sort(images_in_directory.begin(), images_in_directory.end());

		std::vector<std::future<ImageAndName>> results;

		ThreadPool thread_pool { 8 };
		for (const auto& entry : images_in_directory) {
			const auto image_name = remove_extension<std::filesystem::path>(entry);

			results.push_back(thread_pool.push([&](int) {
				const Alabaster::Image image = Alabaster::Image(entry);
				return ImageAndName { image_name, std::move(image) };
			}));
		}

		for (auto& res : results) {
			res.wait();
		}

		for (auto& res : results) {
			auto&& code = res.get();

			images.try_emplace(code.name, code.image);
		}
	}

	template class ImageCache<Alabaster::Image>;

} // namespace AssetManager
