#pragma once

#include <fmt/format.h>

namespace Utilities {

	enum class OutputSize : uint8_t { B = 0, KB = 1, MB = 2, GB = 3, TB = 4 };

	template <OutputSize output_size = OutputSize::KB> auto inline human_readable_size(auto size_bytes)
	{
		const auto to_float = static_cast<float>(size_bytes);

		if constexpr (output_size == OutputSize::KB) {
			return fmt::format("{}kB", to_float / 1000.0f);
		}
		if constexpr (output_size == OutputSize::MB) {
			return fmt::format("{}mB", to_float / 1e6f);
		}
		if constexpr (output_size == OutputSize::GB) {
			return fmt::format("{}gB", to_float / 1e8);
		}
		if constexpr (output_size == OutputSize::TB) {
			return fmt::format("{}tB", to_float / 1e12);
		}
		return fmt::format("{}B", to_float);
	}

} // namespace Utilities
