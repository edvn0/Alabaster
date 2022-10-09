#pragma once

#include "core/Logger.hpp"

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
		if (!static_cast<bool>(happy)) {
			Log::error("Verification failed.");
		}
	}

	static constexpr auto enum_name = [](auto&& in) { return magic_enum::enum_name(in); };

} // namespace Alabaster
