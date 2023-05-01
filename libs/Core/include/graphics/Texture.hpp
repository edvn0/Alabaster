//
// Created by Edwin Carlsson on 2022-10-25.
//

#pragma once

#include "core/Buffer.hpp"
#include "filesystem/FileSystem.hpp"
#include "graphics/Image.hpp"

#include <filesystem>
#include <string>

namespace Alabaster {

	class Texture {
	public:
		~Texture();
		void resize(const glm::uvec2& size);
		void resize(std::uint32_t width, uint32_t height);

		void invalidate();

		ImageFormat get_format() const { return format; }
		std::uint32_t get_width() const { return width; }
		std::uint32_t get_height() const { return height; }
		glm::uvec2 get_size() const { return { width, height }; }

		const std::shared_ptr<Image>& get_image() const { return image; }
		const VkDescriptorImageInfo& get_descriptor_info() const;

		Buffer get_writeable_buffer();
		bool loaded() const { return image_data; }
		const std::filesystem::path& get_path() const;
		std::uint32_t get_mip_level_count() const;
		std::pair<std::uint32_t, uint32_t> get_mip_size(uint32_t mip) const;

		void generate_mips();

		uint64_t get_hash() const;

	private:
		/// @brief Loads the image data from disk, sets up width, height and format.
		/// @param path path of the texture
		/// @return false if could not load the data
		bool load_image(const std::string& path);

		/// @brief Loads the image data from memory, sets up width, height and format.
		/// @param path path of the texture
		/// @return false if could not load the data
		bool load_image(const void* data, std::uint32_t size);

		std::filesystem::path path;
		std::uint32_t width;
		std::uint32_t height;
		TextureProperties properties;

		Buffer image_data;

		std::shared_ptr<Image> image;

		ImageFormat format = ImageFormat::None;

		Texture(const std::filesystem::path& path, TextureProperties properties);
		Texture(ImageFormat format, std::uint32_t width, uint32_t height, const void* data, TextureProperties properties);
		explicit Texture(const void* data, std::size_t size);

	public:
		/// @brief Creates a Texture from a texture filename. This assumes that the filename is found in %root%/textures/{filename}.
		/// @tparam T std::string or std::filesystem::path
		/// @param filename name and extension (image.jpg, image.png etc)
		/// @return constructed and available Texture
		template <typename T> static std::shared_ptr<Texture> from_filename(const T& filename)
		{
			static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::filesystem::path>);
			if constexpr (std::is_same_v<T, std::filesystem::path>)
				return std::shared_ptr<Texture>(new Texture { FileSystem::texture(filename), TextureProperties(filename.string()) });
			else
				return std::shared_ptr<Texture>(new Texture { FileSystem::texture(filename), TextureProperties(filename) });
		}

		/// @brief Creates a Texture from full path like %root%/editor/some_icon.png
		/// @tparam T std::string or std::filesystem::path
		/// @param filename full path like <root>/editor/some_icon.png
		/// @return constructed and available Texture
		template <typename T> static std::shared_ptr<Texture> from_full_path(const T& filename)
		{
			static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::filesystem::path>);
			if constexpr (std::is_same_v<T, std::filesystem::path>)
				return std::shared_ptr<Texture>(new Texture { filename, TextureProperties(filename.string()) });
			else
				return std::shared_ptr<Texture>(new Texture { filename, TextureProperties(filename) });
		}

		static std::shared_ptr<Texture> from_filename(const std::filesystem::path& filename, const TextureProperties& props);

		template <std::size_t Size> static std::shared_ptr<Texture> from_data(const void* data)
		{
			return std::shared_ptr<Texture>(new Texture { data, Size });
		}
	};

} // namespace Alabaster
