//
// Created by Edwin Carlsson on 2022-10-25.
//

#include "graphics/Texture.hpp"

#include "graphics/Allocator.hpp"
#include "graphics/Buffer.hpp"

#include <stb_image.h>

namespace Alabaster {

	Texture::Texture(void* data, std::size_t size)
	{
		Allocator allocator("Texture");
		Buffer buffer(data, static_cast<uint32_t>(size));
		VkImageCreateInfo image_create_info {};
	}

	Texture::Texture(std::string file_path)
		: path(IO::slashed_to_fp(std::move(file_path)))
	{
		Log::info("Texture found at path: {}", path.string());
		int w, h, channels;

		if (stbi_is_hdr(path.string().c_str())) {
			image_data.data = (byte*)stbi_loadf(path.string().c_str(), &w, &h, &channels, 4);
			image_data.size = width * height * 4 * sizeof(float);
			format = ImageFormat::RGBA;
		} else {
			// stbi_set_flip_vertically_on_load(1);
			image_data.data = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
			image_data.size = width * height * 4;
			format = ImageFormat::RGBA;
		}

		if (!image_data) {
			Log::error("Could not load texture data.");
			throw AlabasterException("TextureLoad.");
		}

		width = static_cast<uint32_t>(w);
		height = static_cast<uint32_t>(h);
	};

} // namespace Alabaster
