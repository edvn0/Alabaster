#pragma once

#include <optional>
#include <unordered_map>
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

		inline uint32_t graphics_queue_family() { return queues[QueueType::Graphics].family; }
		inline VkQueue graphics_queue() { return queues[QueueType::Graphics].queue; }
		inline uint32_t present_queue_family() { return queues[QueueType::Present].family; }
		inline VkQueue present_queue() { return queues[QueueType::Present].queue; }

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

		struct QueueIndices {
			std::optional<uint32_t> graphics;
			std::optional<uint32_t> present;

			bool is_complete() { return graphics && present; }
		};

		enum class QueueType { Graphics = 0, Present };

		struct QueueAndFamily {
			VkQueue queue;
			uint32_t family;
		};

		std::unordered_map<QueueType, QueueAndFamily> queues;
	};

} // namespace Alabaster
