#pragma once

#include "core/Logger.hpp"

#include <array>
#include <debug_break.h>
#include <limits>
#include <magic_enum.hpp>

namespace Alabaster {

	static constexpr auto enum_name = [](auto&& in) { return magic_enum::enum_name(in); };
	static constexpr auto non_empty = [](const auto& in) { return not in.empty(); };

	template <typename U> U& as(auto&& in) { return *static_cast<U*>(in.get()); }
	template <typename U> U& as(const auto& in) { return *static_cast<U*>(in.get()); }

#ifdef ALABASTER_DEBUG

	template <typename VkResult> static constexpr auto vk_check(VkResult result) -> void
	{
		VkResult err = result;
		if (err) {
			Log::info("[VkCheck] Vulkan failed with error: {}", enum_name(result));
			debug_break();
		}
	};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Verification failed.");
		}
	};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Verification failed. Message: {}", message);
		}
	};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Assertion failed.");
			debug_break();
		}
	};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Assertion failed. Message: {}", message);
			debug_break();
		}
	};

#else

	template <typename VkResult> static constexpr auto vk_check(VkResult result) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy, std::string_view message) -> void {};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&& happy) -> void {};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&& happy, std::string_view message) -> void {};

#endif

} // namespace Alabaster