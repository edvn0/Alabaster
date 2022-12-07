#include "av_pch.hpp"

#include "graphics/Framebuffer.hpp"

#include "core/Application.hpp"
#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "core/Window.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"
#include "vulkan/vulkan_core.h"

#include <memory>

namespace Alabaster {

	namespace Utils {
		auto is_depth_format(ImageFormat format) { return format == ImageFormat::Depth32 || format == ImageFormat::Depth24Stencil8; }

		auto vulkan_image_format(ImageFormat in)
		{
			switch (in) {
			case ImageFormat::RGBA:
				return VK_FORMAT_R8G8B8A8_SRGB;
			case ImageFormat::RGB:
				return VK_FORMAT_R8G8B8_SRGB;
			case ImageFormat::SINGLE_CHANNEL:
				return VK_FORMAT_R8_SRGB;
			case ImageFormat::Depth32:
				return VK_FORMAT_D32_SFLOAT;
			case ImageFormat::Depth24Stencil8:
				return VK_FORMAT_D24_UNORM_S8_UINT;

			default:
				throw AlabasterException("We do not support RBG/RBGA.");
			};
		}
	}; // namespace Utils

	Framebuffer::Framebuffer(const FramebufferSpecification& specification)
		: spec(specification)
	{
		if (spec.width == 0) {
			const auto&& [w, h] = Application::the().get_window()->size();
			width = w;
			height = h;
		} else {
			width = spec.width * spec.scale;
			height = spec.height * spec.scale;
		}

		// Create all image objects immediately so we can start referencing them
		// elsewhere
		uint32_t attachment_index = 0;
		if (!spec.existing_framebuffer) {
			for (auto& attachment_specification : spec.attachments) {
				if (spec.existing_image && spec.existing_image->get_properties().layers > 1) {
					if (Utils::is_depth_format(attachment_specification.format)) {
						depth_image = spec.existing_image;
					} else {
						attachment_images.emplace_back(spec.existing_image);
					}
				} else if (spec.existing_images.find(attachment_index) != spec.existing_images.end()) {
					if (!Utils::is_depth_format(attachment_specification.format)) {
						attachment_images.emplace_back(); // This will be set later
					}
				} else if (Utils::is_depth_format(attachment_specification.format)) {
					ImageProps props;
					props.format = attachment_specification.format;
					props.usage = ImageUsage::Attachment;
					props.width = width * spec.scale;
					props.height = height * spec.scale;
					props.debug_name
						= fmt::format("{0}-DepthAttachment{1}", spec.debug_name.empty() ? "Unnamed FB" : spec.debug_name, attachment_index);
					depth_image = Image::create(props);
				} else {
					ImageProps props;
					props.format = attachment_specification.format;
					props.usage = ImageUsage::Attachment;
					props.width = width * spec.scale;
					props.height = height * spec.scale;
					props.debug_name
						= fmt::format("{0}-ColorAttachment{1}", spec.debug_name.empty() ? "Unnamed FB" : spec.debug_name, attachment_index);
					attachment_images.emplace_back(Image::create(props));
				}
				attachment_index++;
			}
		}

		assert_that(!specification.attachments.is_empty());
		resize(width, height, true);
	}

	void Framebuffer::destroy()
	{
		if (frame_buffer) {
			VkFramebuffer framebuffer = frame_buffer;

			const auto device = GraphicsContext::the().device();
			vkDestroyFramebuffer(device, framebuffer, nullptr);

			// Don't free the images if we don't own them
			if (!spec.existing_framebuffer) {
				uint32_t attachment_index = 0;
				for (auto& image : attachment_images) {
					if (spec.existing_images.find(attachment_index) != spec.existing_images.end())
						continue;

					// Only destroy deinterleaved image once and prevent clearing layer views on second framebuffer invalidation
					// if (image->get_properties().layers == 1 || attachment_index == 0 && !image->get_layer_image_view(0))
					//	image->Release();
					attachment_index++;
				}

				if (depth_image) {
					// Do we own the depth image?
					if (spec.existing_images.find((uint32_t)spec.attachments.size() - 1) == spec.existing_images.end())
						depth_image->destroy();
				}
			}
		}
	}

	void Framebuffer::resize(uint32_t w, uint32_t h, bool force_recreate)
	{
		if (!force_recreate && (w == width && h == height))
			return;

		width = w * spec.scale;
		height = h * spec.scale;
		if (!spec.swap_chain_target) {
			invalidate();
		} else {
			auto& sc = Application::the().get_window()->get_swapchain();
			render_pass = sc->get_render_pass();

			clear_values.clear();
			clear_values.emplace_back().color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		}

		for (auto& callback : resize_callbacks)
			callback(*this);
	}

	void Framebuffer::add_resize_callback(const std::function<void(Framebuffer&)>& func) { resize_callbacks.push_back(func); }

	void Framebuffer::invalidate()
	{
		CommandBuffer buffer { 1 };
		buffer.begin();
		destroy();

		Allocator allocator("Framebuffer");
		std::vector<VkAttachmentDescription> attachment_descriptions;
		std::vector<VkAttachmentReference> color_attachment_references;
		VkAttachmentReference depth_attachment_reference;
		clear_values.resize(spec.attachments.size());
		bool create_images = attachment_images.empty();

		if (spec.existing_framebuffer)
			attachment_images.clear();

		uint32_t attachment_index = 0;
		for (auto attachment_specification : spec.attachments) {
			if (Utils::is_depth_format(attachment_specification.format)) {
				if (spec.existing_image) {
					depth_image = spec.existing_image;
				} else if (spec.existing_framebuffer) {
					auto existing_framebuffer = spec.existing_framebuffer;
					depth_image = existing_framebuffer->get_depth_image();
				} else if (spec.existing_images.find(attachment_index) != spec.existing_images.end()) {
					auto existing_image = spec.existing_images.at(attachment_index);
					assert_that(
						Utils::is_depth_format(existing_image->get_properties().format), "Trying to attach non-depth image as depth attachment");
					depth_image = existing_image;
				} else {
					auto depth_attachment_image = depth_image;
					auto& depth_props = depth_attachment_image->get_properties();
					depth_props.width = width * spec.scale;
					depth_props.height = height * spec.scale;
					depth_attachment_image->invalidate(buffer); // Create immediately
				}

				VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
				attachment_description.flags = 0;
				attachment_description.format = Utils::vulkan_image_format(attachment_specification.format);
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = spec.clear_depth_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: if sampling, needs to be store (otherwise DONT_CARE is fine)
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.initialLayout
					= spec.clear_depth_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				if (attachment_specification.format == ImageFormat::Depth24Stencil8
					|| true) // Separate layouts requires a "separate layouts" flag to be enabled
				{
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO: if not sampling
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // TODO: if sampling
					depth_attachment_reference = { attachment_index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
				} else {
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL; // TODO: if not sampling
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL; // TODO: if sampling
					depth_attachment_reference = { attachment_index, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL };
				}
				clear_values[attachment_index].depthStencil = { spec.depth_clear_value, 0 };
			} else {
				// HZ_CORE_ASSERT(!spec.existing_image, "Not supported for color attachments");

				std::shared_ptr<Image> color_attachment;
				if (spec.existing_framebuffer) {
					auto existing_framebuffer = spec.existing_framebuffer;
					const auto existing_image = existing_framebuffer->get_image(attachment_index);
					color_attachment = attachment_images.emplace_back(existing_image);
				} else if (spec.existing_images.find(attachment_index) != spec.existing_images.end()) {
					auto existing_image = spec.existing_images[attachment_index];
					assert_that(!Utils::is_depth_format(existing_image->get_properties().format), "Trying to attach depth image as color attachment");
					color_attachment = existing_image;
					attachment_images[attachment_index] = existing_image;
				} else {
					if (create_images) {
						ImageProps props;
						props.format = attachment_specification.format;
						props.usage = ImageUsage::Attachment;
						props.width = width * spec.scale;
						props.height = height * spec.scale;
						color_attachment = attachment_images.emplace_back(Image::create(props));
					} else {
						auto image = attachment_images[attachment_index];
						auto& props = image->get_properties();
						props.width = width * spec.scale;
						props.height = height * spec.scale;
						color_attachment = image;
						if (color_attachment->get_properties().layers == 1)
							color_attachment->invalidate(buffer);
						else if (attachment_index == 0 && spec.existing_image_layers[0] == 0) {
							color_attachment->invalidate(buffer); // Create immediately
							// colorAttachment->RT_CreatePerSpecificLayerImageViews(spec.ExistingImageLayers);
						}
						// } else if (attachment_index == 0) {
						// 	//colorAttachment->RT_CreatePerSpecificLayerImageViews(spec.ExistingImageLayers);
						// }
					}
				}

				VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
				attachment_description.flags = 0;
				attachment_description.format = Utils::vulkan_image_format(attachment_specification.format);
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = spec.clear_colour_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.initialLayout
					= spec.clear_colour_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				const auto& clear_color = spec.clear_colour;
				clear_values[attachment_index].color = { { clear_color.r, clear_color.g, clear_color.b, clear_color.a } };
				color_attachment_references.emplace_back(VkAttachmentReference { attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}

			attachment_index++;
		}

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = uint32_t(color_attachment_references.size());
		subpass_description.pColorAttachments = color_attachment_references.data();
		if (depth_image)
			subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

		std::vector<VkSubpassDependency> dependencies;
		if (attachment_images.size()) {
			{
				VkSubpassDependency& dependency = dependencies.emplace_back();
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
			{
				VkSubpassDependency& dependency = dependencies.emplace_back();
				dependency.srcSubpass = 0;
				dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		if (depth_image) {
			{
				VkSubpassDependency& dependency = dependencies.emplace_back();
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			{
				VkSubpassDependency& dependency = dependencies.emplace_back();
				dependency.srcSubpass = 0;
				dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
				dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
		render_pass_info.pAttachments = attachment_descriptions.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
		render_pass_info.pDependencies = dependencies.data();

		vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &render_pass_info, nullptr, &render_pass));

		std::vector<VkImageView> attachments(attachment_images.size());
		for (uint32_t i = 0; i < attachment_images.size(); i++) {
			auto image = attachment_images[i];
			if (image->get_properties().layers > 1) {
				// attachments[i] = image->GetLayerImageView(spec.ExistingImageLayers[i]);
			} else {
				attachments[i] = image->get_view();
			}
			assert_that(attachments[i]);
		}

		if (depth_image) {
			auto image = depth_image;
			if (spec.existing_image) {
				assert_that(spec.existing_image_layers.size() == 1, "Depth attachments do not support deinterleaving");
				// attachments.emplace_back(image->GetLayerImageView(spec.ExistingImageLayers[0]));
			} else
				attachments.emplace_back(image->get_view());

			assert_that(attachments.back());
		}
		buffer.end();
		buffer.submit();

		VkFramebufferCreateInfo framebuffer_create_info = {};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = render_pass;
		framebuffer_create_info.attachmentCount = uint32_t(attachments.size());
		framebuffer_create_info.pAttachments = attachments.data();
		framebuffer_create_info.width = width;
		framebuffer_create_info.height = height;
		framebuffer_create_info.layers = 1;

		vk_check(vkCreateFramebuffer(GraphicsContext::the().device(), &framebuffer_create_info, nullptr, &frame_buffer));
	}

} // namespace Alabaster
