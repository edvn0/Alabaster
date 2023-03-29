#include "watcher/FileWatcher.hpp"

#include <algorithm>
#include <execution>

namespace AssetManager {

	void FileWatcher::for_each(const FileInformation& info, const std::vector<std::function<void(const FileInformation&)>>& vec)
	{
		std::for_each(std::execution::par, std::begin(vec), std::end(vec), [&info = info](const auto& func) { func(info); });
	}

} // namespace AssetManager
