#include "av_pch.hpp"

#include "graphics/Framebuffer.hpp"

#include "core/Application.hpp"
#include "core/Common.hpp"
#include "core/Window.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"

#include <memory>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Alabaster {

	Framebuffer::Framebuffer(const FramebufferSpecification& specification)
		: spec(specification)
	{
		if (spec.width == 0 || spec.height == 0) {
			const auto&& [w, h] = Application::the().get_window().size();
			width = w;
			height = h;
		} else {
			width = static_cast<std::uint32_t>(spec.width * spec.scale);
			height = static_cast<std::uint32_t>(spec.height * spec.scale);
		}

		static constexpr auto initialise_attachments = [](const auto& w, const auto& h, const auto& fb_spec, const auto& attachment_specification,
														   auto& fb_images, auto& fb_depth_image, auto& attachment_index) {
			if (fb_spec.existing_image && fb_spec.existing_image->get_specification().layers > 1) {
				if (Utilities::is_depth_format(attachment_specification.format)) {
					fb_depth_image = fb_spec.existing_image;
				} else {
					fb_images.emplace_back(fb_spec.existing_image);
				}
			} else if (fb_spec.existing_images.find(attachment_index) != fb_spec.existing_images.end()) {
				if (!Utilities::is_depth_format(attachment_specification.format)) {
					fb_images.emplace_back();
				}
			} else if (Utilities::is_depth_format(attachment_specification.format)) {
				ImageSpecification props;
				props.format = attachment_specification.format;
				props.usage = ImageUsage::Attachment;
				props.width = static_cast<std::uint32_t>(w * fb_spec.scale);
				props.height = static_cast<std::uint32_t>(h * fb_spec.scale);
				props.debug_name
					= fmt::format("{0}-DepthAttachment{1}", fb_spec.debug_name.empty() ? "Unnamed FB" : fb_spec.debug_name, attachment_index);
				fb_depth_image = Image::create(props);
			} else {
				ImageSpecification props;
				props.format = attachment_specification.format;
				props.usage = ImageUsage::Attachment;
				props.width = static_cast<std::uint32_t>(w * fb_spec.scale);
				props.height = static_cast<std::uint32_t>(h * fb_spec.scale);
				props.debug_name
					= fmt::format("{0}-ColorAttachment{1}", fb_spec.debug_name.empty() ? "Unnamed FB" : fb_spec.debug_name, attachment_index);
				fb_images.emplace_back(Image::create(props));
			}
			attachment_index++;
		};

		if (!spec.existing_framebuffer) {
			uint32_t attachment_index = 0;
			for (const auto& attachment_specification : spec.attachments) {
				initialise_attachments(width, height, spec, attachment_specification, attachment_images, depth_image, attachment_index);
			}
		}

		assert_that(!specification.attachments.is_empty());
		resize(width, height, true);
	}

	void Framebuffer::release()
	{
		if (!frame_buffer) {
			return;
		}

		VkFramebuffer framebuffer = frame_buffer;

		const auto& device = GraphicsContext::the().device();
		vkDestroyFramebuffer(device, framebuffer, nullptr);
		vkDestroyRenderPass(device, render_pass, nullptr);

		Log::info("[Framebuffer] Destroying framebuffer.");

		if (spec.existing_framebuffer)
			return;

		uint32_t attachment_index = 0;
		for (const auto& image : attachment_images) {
			if (spec.existing_images.contains(attachment_index))
				continue;

			if (image->get_specification().layers == 1 || attachment_index == 0) {
				image->release();
				Log::info("[Framebuffer] Destroying framebuffer image.");
			}
			attachment_index++;
		}

		if (depth_image) {
			const auto potential_attachment_index = static_cast<std::uint32_t>(spec.attachments.size() - 1);
			if (!spec.existing_images.contains(potential_attachment_index)) {
				depth_image->release();
				Log::info("[Framebuffer] Destroying framebuffer depth image.");
			}
		}
	}

	void Framebuffer::resize(uint32_t w, uint32_t h, bool force_recreate)
	{
		const auto size_was_changed = w == width && h == height;
		if (!force_recreate && size_was_changed)
			return;

		width = static_cast<std::uint32_t>(w * spec.scale);
		height = static_cast<std::uint32_t>(h * spec.scale);
		if (!spec.swap_chain_target) {
			invalidate();
		} else {
			auto& sc = Application::the().get_window().get_swapchain();
			render_pass = sc.get_render_pass();

			clear_values.clear();
			clear_values.emplace_back().emplace<ColourValue>(std::array<float, 4> { 0, 0, 0, 1.0f });
		}

		for (const auto& callback : resize_callbacks)
			callback(*this);
	}

	void Framebuffer::add_resize_callback(const std::function<void(Framebuffer&)>& func) { resize_callbacks.push_back(func); }

	void Framebuffer::invalidate()
	{
		release();

		Allocator allocator("Framebuffer");
		std::vector<VkAttachmentDescription> attachment_descriptions;
		std::vector<VkAttachmentReference> color_attachment_references;
		VkAttachmentReference depth_attachment_reference;
		clear_values.resize(spec.attachments.size());
		bool create_images = attachment_images.empty();

		if (spec.existing_framebuffer)
			attachment_images.clear();

		uint32_t attachment_index = 0;
		for (auto const& attachment_specification : spec.attachments) {
			if (Utilities::is_depth_format(attachment_specification.format)) {
				if (spec.existing_image) {
					depth_image = spec.existing_image;
				} else if (spec.existing_framebuffer) {
					const auto& existing_framebuffer = spec.existing_framebuffer;
					depth_image = existing_framebuffer->get_depth_image();
				} else if (spec.existing_images.find(attachment_index) != spec.existing_images.end()) {
					const auto& existing_image = spec.existing_images.at(attachment_index);
					assert_that(Utilities::is_depth_format(existing_image->get_specification().format),
						"Trying to attach non-depth image as depth attachment");
					depth_image = existing_image;
				} else {
					const auto& depth_attachment_image = depth_image;
					auto& depth_props = depth_attachment_image->get_specification();
					depth_props.width = static_cast<std::uint32_t>(width * spec.scale);
					depth_props.height = static_cast<std::uint32_t>(height * spec.scale);
					depth_attachment_image->invalidate();
				}

				VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
				attachment_description.flags = 0;
				attachment_description.format = Utilities::vulkan_image_format(attachment_specification.format);
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = spec.clear_depth_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.initialLayout
					= spec.clear_depth_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				if (attachment_specification.format == ImageFormat::DEPTH24STENCIL8 || true) {
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					depth_attachment_reference = { attachment_index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
				} else {
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
					depth_attachment_reference = { attachment_index, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL };
				}
				clear_values[attachment_index] = DepthStencilValue { spec.depth_clear_value, 0 };

			} else {
				std::shared_ptr<Image> color_attachment;
				if (spec.existing_framebuffer) {
					const auto& existing_framebuffer = spec.existing_framebuffer;
					const auto& existing_image = existing_framebuffer->get_image(attachment_index);
					color_attachment = attachment_images.emplace_back(existing_image);
				} else if (spec.existing_images.contains(attachment_index)) {
					const auto& existing_image = spec.existing_images[attachment_index];
					assert_that(
						!Utilities::is_depth_format(existing_image->get_specification().format), "Trying to attach depth image as color attachment");
					color_attachment = existing_image;
					attachment_images[attachment_index] = existing_image;
				} else {
					if (create_images) {
						ImageSpecification props;
						props.format = attachment_specification.format;
						props.usage = ImageUsage::Attachment;
						props.width = static_cast<std::uint32_t>(width * spec.scale);
						props.height = static_cast<std::uint32_t>(height * spec.scale);
						color_attachment = attachment_images.emplace_back(Image::create(props));
					} else {
						const auto& image = attachment_images[attachment_index];
						auto& props = image->get_specification();
						props.width = static_cast<std::uint32_t>(width * spec.scale);
						props.height = static_cast<std::uint32_t>(height * spec.scale);
						color_attachment = image;
						if (color_attachment->get_specification().layers == 1)
							color_attachment->invalidate();
						else if (attachment_index == 0 && spec.existing_image_layers[0] == 0) {
							color_attachment->invalidate();
							color_attachment->create_per_specific_layer_image_views(spec.existing_image_layers);
						} else if (attachment_index == 0) {
							color_attachment->create_per_specific_layer_image_views(spec.existing_image_layers);
						}
					}
				}

				VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
				attachment_description.flags = 0;
				attachment_description.format = Utilities::vulkan_image_format(attachment_specification.format);
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = spec.clear_colour_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.initialLayout
					= spec.clear_colour_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				const auto& clear_color = spec.clear_colour;
				clear_values[attachment_index].emplace<ColourValue>(
					std::array<float, 4> { clear_color.r, clear_color.g, clear_color.b, clear_color.a });
				color_attachment_references.emplace_back(VkAttachmentReference { attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}

			attachment_index++;
		}

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = static_cast<std::uint32_t>(color_attachment_references.size());
		subpass_description.pColorAttachments = color_attachment_references.data();
		if (depth_image)
			subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

		std::vector<VkSubpassDependency> dependencies;
		if (attachment_images.size()) {
			VkSubpassDependency& dependency = dependencies.emplace_back();
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkSubpassDependency& subpass = dependencies.emplace_back();
			subpass.srcSubpass = 0;
			subpass.dstSubpass = VK_SUBPASS_EXTERNAL;
			subpass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			subpass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			subpass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		if (depth_image) {
			VkSubpassDependency& dependency = dependencies.emplace_back();
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkSubpassDependency& subpass = dependencies.emplace_back();
			subpass.srcSubpass = 0;
			subpass.dstSubpass = VK_SUBPASS_EXTERNAL;
			subpass.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			subpass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			subpass.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			subpass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			subpass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
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
			if (const auto& image = attachment_images[i]; image->get_specification().layers > 1) {
				attachments[i] = image->get_layer_image_view(spec.existing_image_layers[i]);
			} else {
				attachments[i] = image->get_view();
			}
			assert_that(attachments[i]);
		}

		if (depth_image) {
			const auto& image = depth_image;
			if (spec.existing_image) {
				assert_that(spec.existing_image_layers.size() == 1, "Depth attachments do not support deinterleaving");
			} else {
				attachments.emplace_back(image->get_view());
			}

			assert_that(attachments.back());
		}

		VkFramebufferCreateInfo framebuffer_create_info = {};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = render_pass;
		framebuffer_create_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
		framebuffer_create_info.pAttachments = attachments.data();
		framebuffer_create_info.width = width;
		framebuffer_create_info.height = height;
		framebuffer_create_info.layers = 1;

		vk_check(vkCreateFramebuffer(GraphicsContext::the().device(), &framebuffer_create_info, nullptr, &frame_buffer));
	}

} // namespace Alabaster
