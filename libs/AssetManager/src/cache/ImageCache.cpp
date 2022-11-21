#include "am_pch.hpp"

#include "cache/ImageCache.hpp"

#include "graphics/CommandBuffer.hpp"
#include "utilities/FileInputOutput.hpp"
#include "utilities/FileSystem.hpp"
#include "utilities/ThreadPool.hpp"

namespace AssetManager {

	template <class T>
	void ImageCache<T>::load_from_directory(const std::filesystem::path& directory, std::unordered_set<std::string> include_extensions)
	{
		using namespace Alabaster;
		auto sorted_images_in_directory = FS::in_directory<std::string>(directory, include_extensions, true);

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
