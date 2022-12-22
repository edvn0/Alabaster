#pragma once

#include <filesystem>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace Alabaster::FS {

	template <typename Output = std::string, bool Recursive = false>
	std::vector<Output> in_directory(const std::filesystem::path& path, std::unordered_set<std::string> extensions, bool sorted)
	{
		static constexpr auto entry_to_string = [](const auto& input) { return input.path().extension().string(); };
		static auto should_include = [&extensions](const auto& input) {
			if (extensions.contains("*")) {
				return true;
			}

			return extensions.contains(entry_to_string(input));
		};
		static auto add_to_output = [](const auto& fd, auto& output) mutable {
			if constexpr (std::is_same_v<Output, std::filesystem::path>) {
				output.push_back(static_cast<Output>(fd.path().string()));
			} else {
				output.push_back(static_cast<Output>(fd.path().string()));
			};
		};

		std::vector<Output> output;
		if constexpr (Recursive) {
			for (const auto& fd : std::filesystem::recursive_directory_iterator { path }) {
				if (should_include(fd)) {
					add_to_output(fd, output);
				}
			}
		} else {
			for (const auto& fd : std::filesystem::directory_iterator { path }) {
				if (should_include(fd)) {
					add_to_output(fd, output);
				}
			}
		}

		if (sorted) {
			std::sort(output.begin(), output.end());
		}

		return output;
	}

} // namespace Alabaster::FS
