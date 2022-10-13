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
		~GraphicsContext() = default;

	public:
		static inline GraphicsContext& the()
		{
			static GraphicsContext context;
			return context;
		}

		void destroy();

		inline VkInstance instance() { return vk_instance; };
		inline VkPhysicalDevice physical_device() { return vk_physical_device; };
		inline VkDevice device() { return vk_device; };

		inline uint32_t graphics_queue_family() { return queues[QueueType::Graphics].family; }
		inline VkQueue graphics_queue() { return queues[QueueType::Graphics].queue; }
		inline uint32_t present_queue_family() { return queues[QueueType::Present].family; }
		inline VkQueue present_queue() { return queues[QueueType::Present].queue; }
		inline uint32_t compute_queue_family() { return queues[QueueType::Compute].family; }
		inline VkQueue compute_queue() { return queues[QueueType::Compute].queue; }

		inline VkCommandPool pool() { return command_pool; };
		inline VkCommandPool compute_pool() { return compute_command_pool; };

		VkCommandBuffer get_command_buffer() { return get_command_buffer(true); };
		VkCommandBuffer get_command_buffer(bool begin) { return get_command_buffer(begin, false); };
		VkCommandBuffer get_command_buffer(bool begin, bool compute);

		void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue);
		void flush_command_buffer(VkCommandBuffer command_buffer) { flush_command_buffer(command_buffer, queues[QueueType::Graphics].queue); };

	private:
		void create_instance();
		void create_device();
		void create_physical_device();
		void find_queue_families();
		void setup_debug_messenger();

		VkInstance vk_instance;
		VkPhysicalDevice vk_physical_device { nullptr };

		VkDevice vk_device { nullptr };
		VkCommandPool command_pool;
		VkCommandPool compute_command_pool;

		VkDebugUtilsMessengerEXT debug_messenger;

		struct QueueIndices {
			std::optional<uint32_t> graphics;
			std::optional<uint32_t> present;
			std::optional<uint32_t> compute;

			bool is_complete() { return graphics && present && compute; }
		};

		enum class QueueType { Graphics = 0, Present, Compute };

		struct QueueAndFamily {
			VkQueue queue;
			uint32_t family;
		};

		std::unordered_map<QueueType, QueueAndFamily> queues;
	};

} // namespace Alabaster
