#pragma once

#include "graphics/Image.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	enum class FramebufferBlendMode { None = 0, OneZero, SrcAlphaOneMinusSrcAlpha, Additive, Zero_SrcColor };

	struct FramebufferTextureSpecification {
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(ImageFormat format)
			: format(format)
		{
		}

		ImageFormat format;
		bool blend = true;
		FramebufferBlendMode blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha;
	};

	struct FramebufferAttachmentSpecification {
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: attachments(attachments)
		{
		}

		auto& operator[](const std::uint32_t index) const { return attachments[index]; }
		auto& operator[](const std::uint32_t index) { return attachments[index]; }

		auto begin() { return attachments.begin(); }
		auto end() { return attachments.end(); }

		auto cbegin() const { return attachments.cbegin(); }
		auto cend() const { return attachments.cend(); }

		auto is_empty() const { return attachments.empty(); }
		auto size() const { return attachments.size(); }

		std::vector<FramebufferTextureSpecification> attachments;
	};

	class Framebuffer;

	struct FramebufferSpecification {
		uint32_t width { 0 };
		uint32_t height { 0 };
		float scale { 1.0f };

		float depth_clear_value = 1.0f;
		bool clear_colour_on_load = true;
		bool clear_depth_on_load = true;
		glm::vec4 clear_colour = { 0.0f, 0.0f, 0.0f, 1.0f };

		FramebufferAttachmentSpecification attachments;
		uint32_t samples { 1 };

		bool no_resize { false };
		bool blend { true };
		FramebufferBlendMode blend_mode { FramebufferBlendMode::None };

		bool swap_chain_target { false };

		std::shared_ptr<Image> existing_image;
		std::vector<uint32_t> existing_image_layers;
		std::map<uint32_t, std::shared_ptr<Image>> existing_images;
		std::shared_ptr<Framebuffer> existing_framebuffer;
		std::string debug_name;
	};

	class Framebuffer {
	public:
		explicit Framebuffer(const FramebufferSpecification& w);
		~Framebuffer()
		{
			if (!destroyed)
				destroy();
		}

		void resize(uint32_t width, uint32_t height, bool force_recreate = false);
		void add_resize_callback(const std::function<void(Framebuffer&)>& func);

		std::uint32_t get_width() const { return width; };
		std::uint32_t get_height() const { return height; };

		const std::shared_ptr<Image>& get_image(std::uint32_t index = 0) const { return attachment_images[index]; }
		const std::shared_ptr<Image>& get_depth_image() const { return depth_image; }

		size_t get_colour_attachment_count() const { return spec.swap_chain_target ? 1 : attachment_images.size(); }
		bool has_depth_attachment() const { return depth_image != nullptr; }

		const VkRenderPass& get_renderpass() const { return render_pass; }
		const VkFramebuffer& get_framebuffer() const { return frame_buffer; }

		const std::vector<VkClearValue>& get_clear_values() const { return clear_values; }
		const FramebufferSpecification& get_specification() const { return spec; }

		void invalidate();
		void release();
		void destroy();

	private:
		FramebufferSpecification spec;
		std::uint32_t width { 0 };
		std::uint32_t height { 0 };

		std::vector<std::shared_ptr<Image>> attachment_images;
		std::shared_ptr<Image> depth_image;

		std::vector<VkClearValue> clear_values;

		VkRenderPass render_pass { nullptr };
		VkFramebuffer frame_buffer { nullptr };

		std::vector<std::function<void(Framebuffer&)>> resize_callbacks;

		bool destroyed { false };

	public:
		static std::shared_ptr<Framebuffer> create(const FramebufferSpecification& spec) { return std::make_shared<Framebuffer>(spec); }
	};

} // namespace Alabaster
