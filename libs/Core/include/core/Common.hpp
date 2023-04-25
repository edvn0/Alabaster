#pragma once

#include "core/Logger.hpp"
#include "core/exceptions/AlabasterException.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <magic_enum.hpp>
#include <string_view>

#ifndef ALABASTER_MACOS
#include <bit>
#endif

#ifdef SUPPORT_EXHAUSTED_EXT
#undef SUPPORT_EXHAUSTED_EXT
#endif

namespace Alabaster {

	void stop();

	static constexpr auto is_equals_ignore_case(const auto* x, const auto* y)
	{
#ifdef ALABASTER_WINDOWS
		return _stricmp(x, y);
#else
		return _strcasecmp(x, y);
#endif
	}

	static constexpr auto str_is_equals_ignore_case(const std::filesystem::path& x, auto&& y)
	{
#ifdef ALABASTER_WINDOWS
		return _stricmp(x.string().data(), y);
#else
		return _strcasecmp(x.string().data(), y);
#endif
	}

	static constexpr auto str_is_equals_ignore_case(const std::string& x, auto&& y)
	{
#ifdef ALABASTER_WINDOWS
		return _stricmp(x.data(), y);
#else
		return _strcasecmp(x.data(), y);
#endif
	}

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

	template <class T> struct vk_result {
		std::string_view operator()(auto&) { return ""; }
	};

	template <typename T>
	concept has_empty = requires(T t) {
		{
			t.empty()
		} -> std::same_as<bool>;
	};
	static constexpr auto non_empty = [](const has_empty auto& in) { return not in.empty(); };

	template <typename T> static constexpr auto reinterpret_as(auto in)
	{
#ifndef ALABASTER_MACOS
		return std::bit_cast<T>(in);
#else
		return reinterpret_cast<T>(in);
#endif
	}

#ifdef ALABASTER_DEBUG

	template <typename VkResult> static inline constexpr auto vk_check(VkResult result) -> decltype(auto)
	{
		VkResult err = result;
		if (err) {
			Log::info("[VkCheck] Vulkan failed with error: {}", vk_result<VkResult> {}(result));
			stop();
		}
	}

	template <typename PositiveCondition> static inline constexpr auto verify(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Verification failed.");
		}
	}

	template <typename PositiveCondition> static inline constexpr auto verify(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Verification failed. Message: {}", message);
		}
	}

	template <typename PositiveCondition> static inline constexpr auto assert_that(PositiveCondition&& happy) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Assertion failed.");
			stop();
		}
	}

	template <typename PositiveCondition> static inline constexpr auto assert_that(PositiveCondition&& happy, std::string_view message) -> void
	{
		auto result = static_cast<bool>(happy);
		if (not result) {
			Log::error("Assertion failed. Message: {}", message);
			stop();
		}
	}

#else

	template <typename VkResult> static constexpr auto vk_check(VkResult) -> void { }

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&&) -> void { }

	template <typename PositiveCondition> static constexpr auto verify(PositiveCondition&&, std::string_view) -> void { }

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&&) -> void { }

	template <typename PositiveCondition> static constexpr auto assert_that(PositiveCondition&&, std::string_view) -> void { }

#endif

} // namespace Alabaster
