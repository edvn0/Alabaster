#pragma once

#include <cmath>
#include <fmt/format.h>

namespace Alabaster::Utilities {

	template <typename T> auto split_into(const std::vector<T>& items, std::uint32_t batch_size = 8)
	{
		std::vector<std::vector<T>> batches;
		const auto max_batches = ceilf(items.size() / static_cast<float>(batch_size));

		batches.resize(static_cast<std::uint32_t>(max_batches));
		for (size_t i = 0; i < items.size(); i += batch_size) {
			auto last = std::min(items.size(), i + batch_size);
			auto index = i / batch_size;
			auto& vec = batches[index];
			vec.reserve(last - i);
			std::move(items.begin() + i, items.begin() + last, back_inserter(vec));
		}
		return batches;
	}

	enum class OutputSize : uint8_t { B = 0, KB = 1, MB = 2, GB = 3, TB = 4 };

	static constexpr std::size_t threshold_b_to_kb = 1000;
	static constexpr std::size_t threshold_b_to_mb = 1'000'000;
	static constexpr std::size_t threshold_b_to_gb = 1'000'000'000;

	template <OutputSize output_size = OutputSize::KB> auto human_readable_size(auto size_bytes)
	{
		const auto to_float = static_cast<float>(size_bytes);

		if (size_bytes > threshold_b_to_gb) {
			return fmt::format("{}gB", to_float / 1e8);
		} else if (size_bytes > threshold_b_to_mb) {
			return fmt::format("{}mB", to_float / 1e6f);
		} else if (size_bytes > threshold_b_to_kb) {
			return fmt::format("{}kB", to_float / 1000.0f);
		} else {
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
		}
	}
} // namespace Alabaster::Utilities
