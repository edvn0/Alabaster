//
// Created by Edwin Carlsson on 2022-10-25.
//

#pragma once

#include "core/Buffer.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/CommandBuffer.hpp"
#include "utilities/FileInputOutput.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <optional>
#include <string>

using VkImage = struct VkImage_T*;
using VkImageView = struct VkImageView_T*;
using VkSampler = struct VkSampler_T*;
using VmaAllocation = struct VmaAllocation_T*;

struct VkDescriptorImageInfo;

namespace Alabaster {

	enum class ImageFormat {
		None = 0,
		RED8UN,
		RED8UI,
		RED16UI,
		RED32UI,
		RED32F,
		RG8,
		RG16F,
		RG32F,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,

		B10R11G11UF,

		SRGB,

		DEPTH32FSTENCIL8UINT,
		DEPTH32F,
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	enum class ImageUsage { None = 0, Texture, Attachment, Storage };

	enum class TextureWrap { None = 0, Clamp, Repeat };

	enum class TextureFilter { None = 0, Linear, Nearest, Cubic };

	enum class TextureType { None = 0, Texture2D, TextureCube };

	struct TextureProperties {
		explicit TextureProperties(const std::string& name = "Debug")
			: debug_name(name) {};

		std::string debug_name;
		TextureWrap sampler_wrap { TextureWrap::Repeat };
		TextureFilter sampler_filter { TextureFilter::Linear };
		bool generate_mips { true };
		bool srgb { false };
		bool storage { false };
	};

	struct ImageSpecification {
		std::string debug_name;

		ImageFormat format = ImageFormat::RGBA;
		ImageUsage usage = ImageUsage::Texture;
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t mips = 1;
		uint32_t layers = 1;
	};

	struct ImageInfo {
		VkImage image { nullptr };
		VkImageView view { nullptr };
		VkSampler sampler { nullptr };
		VmaAllocation allocation { nullptr };
	};

	class Image {
	public:
		explicit Image(const ImageSpecification& specification) noexcept;

		void destroy();
		void resize(const uint32_t width, const uint32_t height)
		{
			spec.width = width;
			spec.height = height;
			invalidate();
		}
		void resize(const glm::uvec2& size) { resize(size.x, size.y); }
		void invalidate();
		void release();

		uint32_t get_width() const { return spec.width; };
		uint32_t get_height() const { return spec.height; };
		glm::uvec2 get_size() const { return { spec.width, spec.height }; };

		float get_aspect_ratio() const { return static_cast<float>(spec.width) / static_cast<float>(spec.height); };

		ImageSpecification& get_specification() { return spec; };
		const ImageSpecification& get_specification() const { return spec; };

		const auto& get_view() const { return info.view; }
		const auto& get_sampler() const { return info.sampler; }
		const auto& get_image() const { return info.image; }
		auto& get_info() const { return info; }
		auto& get_info() { return info; }

		void create_per_layer_image_view();
		VkImageView get_layer_image_view(std::uint32_t layer);
		VkImageView get_mip_image_view(std::uint32_t mip);

		Buffer get_buffer() const { return image_data; }
		Buffer& get_buffer() { return image_data; }

		uint64_t get_hash() const;

		void update_descriptor();

		const VkDescriptorImageInfo& get_descriptor_info() const;
		void create_per_specific_layer_image_views(const std::vector<std::uint32_t>& layer_indices);

	private:
		ImageSpecification spec;

		Buffer image_data;

		ImageInfo info;

		std::vector<VkImageView> per_layer_image_views;
		std::map<uint32_t, VkImageView> per_mip_image_views;
		std::unique_ptr<VkDescriptorImageInfo> descriptor_image_info;

		bool destroyed { false };

	public:
		static std::shared_ptr<Image> create(ImageSpecification spec) { return std::make_shared<Image>(spec); }
	};

} // namespace Alabaster
