//
// Created by Edwin Carlsson on 2022-10-25.
//

#pragma once

#include "graphics/Buffer.hpp"
#include "graphics/Image.hpp"
#include "utilities/FileInputOutput.hpp"

#include <string>

namespace Alabaster {

	class Texture {
	public:
		explicit Texture(std::string file_path);
		explicit Texture(void* data, std::size_t size);

	private:
		std::filesystem::path path;
		std::uint32_t width { 0 };
		std::uint32_t height { 0 };
		Buffer image_data;
		std::unique_ptr<Image> image;
		ImageFormat format = ImageFormat::RBGA;
	};

	class Texture2D : public Texture {
	public:
		explicit Texture2D(std::string file_path)
			: Texture(std::move(file_path)) {};
		explicit Texture2D(void* data, std::size_t size)
			: Texture(data, size) {};
	};

	class TextureCube : public Texture { };

} // namespace Alabaster