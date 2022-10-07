#pragma once

#include <vulkan/vulkan.h>

namespace Alabaster {

	class GraphicsContext {
	public:
		GraphicsContext(const GraphicsContext&) = delete;
		void operator=(const GraphicsContext&) = delete;
		GraphicsContext(GraphicsContext&&) = delete;

	private:
		GraphicsContext();
		~GraphicsContext();

	public:
		static inline GraphicsContext& the()
		{
			static GraphicsContext context;
			return context;
		}

		inline VkInstance instance() { return vk_instance; };
		inline VkPhysicalDevice physical_device() { return vk_physical_device; };
		inline VkDevice device() { return vk_device; };

		inline uint32_t graphics_queue_family() { return queues.graphics_queue_family; }
		inline VkQueue graphics_queue() { return queues.graphics_queue; }
		inline uint32_t compute_queue_family() { return queues.compute_queue_family; }
		inline VkQueue compute_queue() { return queues.compute_queue; }
		inline uint32_t transfer_queue_family() { return queues.transfer_queue_family; }
		inline VkQueue transfer_queue() { return queues.transfer_queue; }

	private:
		void create_instance();
		void create_device();
		void create_physical_device();
		void find_queue_families();
		void setup_debug_messenger();

		VkInstance vk_instance;
		VkPhysicalDevice vk_physical_device { nullptr };
		VkDevice vk_device { nullptr };
		VkDebugUtilsMessengerEXT debug_messenger;

		struct {
			VkQueue graphics_queue;
			uint32_t graphics_queue_family;

			VkQueue transfer_queue;
			uint32_t transfer_queue_family;

			VkQueue compute_queue;
			uint32_t compute_queue_family;
		} queues;
	};

} // namespace Alabaster
