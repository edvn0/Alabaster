//
// Created by Edwin Carlsson on 2022-10-25.
//

#include "graphics/Texture.hpp"

#include "graphics/Allocator.hpp"
#include "graphics/Buffer.hpp"

#include <stb_image.h>

namespace Alabaster {

	Texture::Texture(void* data, size_t size)
	{
		Allocator allocator("Texture");
		Buffer buffer(data, size);
		VkImageCreateInfo image_create_info {};
	}

} // namespace Alabaster
