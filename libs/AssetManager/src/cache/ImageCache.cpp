#include "am_pch.hpp"

#include "cache/ImageCache.hpp"

#include "graphics/CommandBuffer.hpp"
#include "utilities/FileInputOutput.hpp"
#include "utilities/ThreadPool.hpp"

namespace AssetManager {

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
		using namespace Alabaster;
		auto sorted_images_in_directory = IO::in_directory<std::string>(shader_directory, { ".tga", ".png", ".jpeg", ".jpg" }, true);

		ThreadPool pool { 8 };
		buffer->begin();
		for (const auto& entry : sorted_images_in_directory) {
			const auto image_name = remove_extension<std::filesystem::path>(entry);
			ImageProps props { .path = entry };
			// Load all images and obtain pixels from image data (png, jpg etc)
			Image image = Image(props);
			// Push a function into the thread pool which should:

			// This is the work horse:
			// Submit into the common (for all images) VkCommandBuffer
			// 1) Move pixel data of image to staging buffer,
			// 2) create VkImage.
			// 3) transition image from undefined to dst_optimal (in same CB)
			// 4) copy pixel data from staging to VkImage (in same CB)
			// 5) transition from dst_optimal to shader_read_optimal (in same CB)
			// 6) Create VkImageView and VkSampler
			image.invalidate(buffer);
			// invalidate also adds a 'destructor' callback to the command buffer, which
			// just prior to vkQueueSubmit deallocates the staging buffer, like so:
			// buffer->add_destruction_callback([&staging_buffer_allocation, &staging_buffer]
			//	(Allocator& allocator) { allocator.destroy_buffer(staging_buffer, staging_buffer_allocation); });

			images.try_emplace(image_name, image); // Push into image/texture cache
		}
		pool.stop();
		buffer->end();
		buffer->submit();
	}

	template class ImageCache<Alabaster::Image>;

} // namespace AssetManager
