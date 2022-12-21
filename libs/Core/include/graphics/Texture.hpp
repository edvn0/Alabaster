//
// Created by Edwin Carlsson on 2022-10-25.
//

#pragma once

#include "core/Buffer.hpp"
#include "graphics/Image.hpp"
#include "utilities/FileInputOutput.hpp"

#include <filesystem>
#include <string>

namespace Alabaster {

	class Texture {
	public:
		Texture(const std::filesystem::path& path, TextureProperties properties);
		Texture(ImageFormat format, std::uint32_t width, uint32_t height, const void* data, TextureProperties properties);
		~Texture();
		void resize(const glm::uvec2& size);
		void resize(std::uint32_t width, uint32_t height);

		void destroy();
		void invalidate();

		ImageFormat get_format() const { return format; }
		std::uint32_t get_width() const { return width; }
		std::uint32_t get_height() const { return height; }
		glm::uvec2 get_size() const { return { width, height }; }

		const std::shared_ptr<Image>& get_image() const { return image; }
		const VkDescriptorImageInfo& get_descriptor_info() const { return image->get_descriptor_info(); }

		Buffer get_writeable_buffer();
		bool loaded() const { return image_data; }
		const std::filesystem::path& get_path() const;
		std::uint32_t get_mip_level_count() const;
		std::pair<std::uint32_t, uint32_t> get_mip_size(uint32_t mip) const;

		void generate_mips();

		uint64_t get_hash() const { return (uint64_t)image->get_descriptor_info().imageView; }

	private:
		bool load_image(const std::string& path);
		bool load_image(const void* data, std::uint32_t size);

	private:
		std::filesystem::path path;
		std::uint32_t width;
		std::uint32_t height;
		TextureProperties properties;

		Buffer image_data;

		std::shared_ptr<Image> image;

		ImageFormat format = ImageFormat::None;

		bool destroyed { false };

	public:
		static std::shared_ptr<Texture> from_filename(auto&& filename)
		{
			return std::make_shared<Texture>(IO::texture(filename), TextureProperties {});
		}

		static std::shared_ptr<Texture> from_filename(auto&& filename, auto&& props)
		{
			return std::make_shared<Texture>(IO::texture(filename), props);
		}
	};

} // namespace Alabaster
