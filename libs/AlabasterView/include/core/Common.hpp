#pragma once

#include "core/Logger.hpp"

#include <debug_break.h>
#include <magic_enum.hpp>

namespace Alabaster {

	template <typename VkResult> static constexpr auto vk_check(VkResult result) -> void
	{
		VkResult err = result;
		if (err) {
			Log::info("Vulkan failed with error: {}", result);
			debug_break();
		}
	};

	template <typename PositiveCondition> auto verify(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (!result) {
			Log::error("Verification failed.");
		}
	}

	template <typename PositiveCondition> auto verify(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (!result) {
			Log::error("Verification failed. Message: {}", message);
		}
	}

	static constexpr auto enum_name = [](auto&& in) { return magic_enum::enum_name(in); };

} // namespace Alabaster
