#pragma once

#include "core/Logger.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace Alabaster::IO {

	std::string read_file(const std::filesystem::path& filename);
	bool exists(const std::filesystem::path& path);

	template <typename T>
	concept Printable = requires(std::ofstream& os, const T& printable)
	{
		{ printable.write_to(os) };
	};

	template <Printable T> bool write_file(const std::filesystem::path& filename, const T& printable)
	{
		std::ofstream output_stream(filename);
		if (!output_stream) {
			Log::error("Could not write output file: {}", filename);
			return false;
		}

		printable.write_to(output_stream);
		return true;
	}

} // namespace Alabaster::IO
