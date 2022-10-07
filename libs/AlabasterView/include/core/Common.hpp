#pragma once

#include "core/Logger.hpp"

namespace Alabaster {

	template <typename VkResult> static constexpr auto vk_check(VkResult result) -> void
	{
		VkResult err = result;
		if (err) {
			Log::info("Vulkan failed with error: {}", result);
			throw std::runtime_error("Vulkan Fail with error");
		}
	};

} // namespace Alabaster
