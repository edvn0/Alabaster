#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace Alabaster::IO {

	std::string read_file(const std::filesystem::path& filename);

} // namespace Alabaster::IO
