#pragma once

#include "core/exceptions/AlabasterException.hpp"
#include "core/Logger.hpp"

#include <debug_break.h>
#include <magic_enum.hpp>

namespace Alabaster {

#ifdef ALABASTER_DEBUG

	template <typename VkResult> static constexpr auto vk_check(VkResult result) -> void
	{
		VkResult err = result;
		if (err) {
			Log::info("Vulkan failed with error: {}", result);
			debug_break();
		}
	};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (!result) {
			Log::error("Verification failed.");
		}
	};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (!result) {
			Log::error("Verification failed. Message: {}", message);
		}
	};

#else

	template <typename VkResult> static constexpr auto vk_check(VkResult result) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&& happy, std::string_view message) -> void {};

#endif

	static constexpr auto enum_name = [](auto&& in) { return magic_enum::enum_name(in); };
	static constexpr auto non_empty = [](const auto& in) { return not in.empty(); };

} // namespace Alabaster
