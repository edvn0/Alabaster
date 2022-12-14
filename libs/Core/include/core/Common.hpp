#pragma once

#include "core/Logger.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <debug_break.h>
#include <limits>
#include <magic_enum.hpp>

namespace Alabaster {

	template <typename T>
	concept HasSizeAndIterator = requires(T t) {
									 t.size();
									 t.begin();
									 t.end();
								 };

	static constexpr auto equals_ignore_case(const HasSizeAndIterator auto& lhs, const HasSizeAndIterator auto& rhs)
	{
		if (lhs.size() != rhs.size())
			return false;
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char a, char b) { return std::tolower(a) == std::tolower(b); });
	}

	static constexpr auto enum_name = [](auto&& in) { return magic_enum::enum_name(in); };
	static constexpr auto vk_result = [](VkResult result) {
		switch (result) {
		case VK_SUCCESS:
			return "VK_SUCCESS";
		case VK_NOT_READY:
			return "VK_NOT_READY";
		case VK_TIMEOUT:
			return "VK_TIMEOUT";
		case VK_EVENT_SET:
			return "VK_EVENT_SET";
		case VK_EVENT_RESET:
			return "VK_EVENT_RESET";
		case VK_INCOMPLETE:
			return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:
			return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:
			return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:
			return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION:
			return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_PIPELINE_COMPILE_REQUIRED:
			return "VK_PIPELINE_COMPILE_REQUIRED";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:
			return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV:
			return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_NOT_PERMITTED_KHR:
			return "VK_ERROR_NOT_PERMITTED_KHR";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR:
			return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR:
			return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR:
			return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
			return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
		default:
			return "Missing VkResult enum mapping";
		}
	};

	template <typename T>
	concept has_empty = requires(T t) { t.empty(); };
	static constexpr auto non_empty = [](const has_empty auto& in) { return not in.empty(); };

#ifdef ALABASTER_DEBUG

	template <typename VkResult> static inline constexpr auto vk_check(VkResult result) -> void
	{
		VkResult err = result;
		if (err) {
			Log::info("[VkCheck] Vulkan failed with error: {}", vk_result(result));
			debug_break();
		}
	};

	template <typename PositiveCondition> static inline constexpr auto verify(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Verification failed.");
		}
	};

	template <typename PositiveCondition> static inline constexpr auto verify(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Verification failed. Message: {}", message);
		}
	};

	template <typename PositiveCondition> static inline constexpr auto assert_that(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Assertion failed.");
			debug_break();
		}
	};

	template <typename PositiveCondition> static inline constexpr auto assert_that(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Assertion failed. Message: {}", message);
			debug_break();
		}
	};

#else

	template <typename VkResult> static constexpr auto vk_check(VkResult) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&&) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&&, std::string_view) -> void {};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&&) -> void {};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&&, std::string_view) -> void {};

#endif

} // namespace Alabaster
