#include "av_pch.hpp"

#include "graphics/GraphicsContext.hpp"

#include "core/Common.hpp"
#include "core/Logger.hpp"

#include <GLFW/glfw3.h>
#include <magic_enum.hpp>

namespace Alabaster {

#ifdef ALABASTER_VALIDATION
	static const std::vector<const char*> requested_validation_layers { "VK_LAYER_KHRONOS_validation" };
#else
	static const std::vector<const char*> requested_validation_layers;
#endif

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* pUserData)
	{
		std::string type = message_type == 1 ? "General" : "1";
		if (message_severity & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
			Log::info("Validation Layer: [Message Type: {}, Message: {}]", type, callback_data->pMessage, callback_data->queueLabelCount);
			return VK_TRUE;
		} else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			Log::info("Validation Layer: [Message Type: {}, Message: {}]", type, callback_data->pMessage, callback_data->queueLabelCount);
			return VK_TRUE;
		} else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			Log::warn("Validation Layer: [Message Type: {}, Message: {}]", type, callback_data->pMessage, callback_data->queueLabelCount);
			return VK_TRUE;
		} else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			Log::error("Validation Layer: [Message Type: {}, Message: {}]", type, callback_data->pMessage, callback_data->queueLabelCount);
			return VK_TRUE;
		} else {
			Log::error("Message Severity: {}", message_severity);
			return VK_FALSE;
		}
	}

	void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debug_messenger, allocator);
		}
	}

	void populate_debug_messenger(VkDebugUtilsMessengerCreateInfoEXT& create_info)
	{
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debug_callback;
	}

	bool check_validation_support()
	{
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		for (const auto& layer : requested_validation_layers) {
			bool found = false;

			for (const auto& layer_properties : available_layers) {
				const bool has_same_name = layer_properties.layerName == layer;

				if (has_same_name) {
					found = true;
					break;
				}
			}

			if (!found) {
				return false;
			}
		}

		return true;
	}

	VkResult create_debug_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info,
		const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, create_info, allocator, debug_messenger);
		} else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	GraphicsContext::GraphicsContext()
	{
		create_instance();
		create_physical_device();
		create_device();
	}

	GraphicsContext::~GraphicsContext()
	{
		destroy_debug_messenger(vk_instance, debug_messenger, nullptr);
		vkDestroyDevice(vk_device, nullptr);
	}

	void GraphicsContext::setup_debug_messenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info {};
		populate_debug_messenger(create_info);

		if (create_debug_messenger(vk_instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void GraphicsContext::create_instance()
	{
		bool enable_layers = !requested_validation_layers.empty();

		if (enable_layers && check_validation_support()) {
			throw std::runtime_error("Validation layer support requested but could not be given.");
		}

		VkApplicationInfo application_info {};
		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pApplicationName = "Alabaster";
		application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.pEngineName = "Alabaster Engine";
		application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo instance_info {};
		instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_info.pApplicationInfo = &application_info;

		uint32_t glfw_ext_count = 0;
		const char** glfw_extensions;

		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_ext_count);

		if (enable_layers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instance_info.ppEnabledExtensionNames = extensions.data();

		// Validation layers
		instance_info.enabledLayerCount = 0;
		instance_info.pNext = nullptr;

		if (enable_layers) {
			instance_info.enabledLayerCount = requested_validation_layers.size();
			instance_info.ppEnabledLayerNames = requested_validation_layers.data();

			VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
			populate_debug_messenger(debug_create_info);
			instance_info.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_create_info);
		}

		vk_check(vkCreateInstance(&instance_info, nullptr, &vk_instance));
		setup_debug_messenger();
	}

	void GraphicsContext::create_device()
	{
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queues = { queues[QueueType::Graphics].family, queues[QueueType::Present].family };

		float queue_prio = 1.0f;
		for (uint32_t family : unique_queues) {
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_prio;
			queue_create_infos.push_back(queue_create_info);
		}

		std::vector<const char*> device_exts;
		device_exts.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		device_exts.push_back("VK_KHR_portability_subset");

		VkDeviceCreateInfo device_create_info {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		device_create_info.pQueueCreateInfos = queue_create_infos.data();
		VkPhysicalDeviceFeatures device_features {};
		device_create_info.pEnabledFeatures = &device_features;

		if (!device_exts.empty()) {
			device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_exts.size());
			device_create_info.ppEnabledExtensionNames = device_exts.data();
		}

		device_create_info.enabledLayerCount = 0;
		if (!requested_validation_layers.empty()) {
			device_create_info.enabledLayerCount = static_cast<uint32_t>(requested_validation_layers.size());
			device_create_info.ppEnabledLayerNames = requested_validation_layers.data();
		}
		vk_check(vkCreateDevice(vk_physical_device, &device_create_info, nullptr, &vk_device));

		vkGetDeviceQueue(vk_device, queues[QueueType::Graphics].family, 0, &queues[QueueType::Graphics].queue);
		vkGetDeviceQueue(vk_device, queues[QueueType::Present].family, 0, &queues[QueueType::Present].queue);
	}

	void GraphicsContext::create_physical_device()
	{
		static constexpr auto is_device_suitable = [](const VkPhysicalDevice& device) -> bool {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);

			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(device, &features);

			return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		};

		uint32_t device_count { 0 };
		vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);

		if (device_count == 0) {
			throw std::runtime_error("Could not find any GPUs.");
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data());
		for (const auto& device : devices) {
			if (is_device_suitable(device)) {
				vk_physical_device = device;
				break;
			}
		}
		Log::info("Physical device chosen.");

		find_queue_families();
		Log::info("Queues: Graphics: [{}], Present: [{}]", queues[QueueType::Graphics].family, queues[QueueType::Present].family);
	}

	void GraphicsContext::find_queue_families()
	{
		static constexpr auto is_complete = [](const auto& qs) {
			return qs.compute_queue_family != 0 && qs.transfer_queue_family != 0 && qs.graphics_queue_family != 0 && qs.present_queue_family != 0;
		};

		QueueIndices indices;

		uint32_t nr_families = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &nr_families, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(nr_families);
		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &nr_families, queue_families.data());

		int family_index = 0;

		for (const auto& queue_family : queue_families) {
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphics = family_index;
				queues[QueueType::Graphics].family = *indices.graphics;
			}

			// The implicit assumption here is that this queue can present...
			indices.present = family_index;
			queues[QueueType::Present].family = *indices.graphics;

			if (indices.is_complete()) {
				break;
			}

			family_index++;
		}
	}

} // namespace Alabaster
