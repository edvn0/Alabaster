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
	static constexpr auto non_empty = [](const auto& in) { return not in.empty(); };

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

	template <typename VkResult> static constexpr auto vk_check(VkResult) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&&) -> void {};

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&&, std::string_view) -> void {};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&&) -> void {};

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&&, std::string_view) -> void {};

#endif

} // namespace Alabaster
