#pragma once

#include "core/Common.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace Alabaster::IO {

	enum class OpenMode : int {
		Read = 0x01,
		Write = 0x02,
		AtEnd = 0x04,
		Append = 0x08,
		Truncate = 0x10,
		NoCreate = 0x40,
		NoReplace = 0x80,
		Binary = 0x20
	};

	constexpr OpenMode operator|(const OpenMode& current, const OpenMode& other)
	{
		return static_cast<OpenMode>(static_cast<int>(current) | static_cast<int>(other));
	}

	std::string read_file(const std::filesystem::path& filename, OpenMode mode = OpenMode::Binary | OpenMode::AtEnd);
	bool exists(const std::filesystem::path& path);
	std::filesystem::path independent_path(const std::string& path);

	// clang-format off

    /**
     * @brief Printable structs are those that define a function bool write_to(std::ofstream& os).
     * 
     * @tparam T instance of Printable.
     */
	template <typename T>
	concept Printable = requires(std::ofstream& os, const T& printable)
	{
		{  printable.write_to(os)  } -> std::same_as<bool>;
	};
	// clang-format on

	template <Printable T> static inline bool write_file(const std::filesystem::path& filename, const T& printable)
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
	}

} // namespace Alabaster::IO
