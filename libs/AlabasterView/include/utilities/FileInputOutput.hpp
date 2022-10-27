#pragma once

#include "core/Common.hpp"
#include "core/OpenMode.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace Alabaster::IO {

	std::filesystem::path resources();
	std::string read_file(const std::filesystem::path& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);
	std::string read_file(std::filesystem::path&& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);
	bool exists(const std::filesystem::path& path);
	bool is_file(const std::filesystem::path& path);
	std::filesystem::path independent_path(const std::string& path);
	std::filesystem::path slashed_to_fp(const std::string& slashed_string);

	template <typename Printable> static inline bool write_file(const std::filesystem::path& filename, Printable& printable)
	{
		std::ofstream output_stream(filename);
		if (!output_stream) {
			Log::error("Could not write output file: {}", filename);
			return false;
		}

		if (printable.write_to(output_stream)) {
			return true;
		}
		Log::error("Could not write to output file stream.");
		return false;
	}

} // namespace Alabaster::IO
