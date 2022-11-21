#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

namespace Alabaster::FS {

	template <typename Output = std::string, bool Recursive = false>
	std::vector<Output> in_directory(const std::filesystem::path& path, std::unordered_set<std::string> extensions, bool sorted)
	{
		static constexpr auto entry_to_string = [](const auto& input) { return input.path().extension().string(); };
		std::vector<Output> output;
		if constexpr (Recursive) {
			for (const auto& fd : std::filesystem::recursive_directory_iterator { path }) {
				if (extensions.count(entry_to_string(fd))) {
					output.push_back(static_cast<Output>(fd.path().string()));
				}
			}
		} else {
			for (const auto& fd : std::filesystem::directory_iterator { path }) {
				if (extensions.count(entry_to_string(fd))) {
					output.push_back(static_cast<Output>(fd.path().string()));
				}
			}
		}

		if (sorted) {
			std::sort(output.begin(), output.end());
		}

		return output;
	}

} // namespace Alabaster::FS