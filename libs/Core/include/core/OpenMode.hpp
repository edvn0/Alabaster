//
// Created by Edwin Carlsson on 2022-10-17.
//

#pragma once

#include "core/exceptions/AlabasterException.hpp"

#include <filesystem>

namespace Alabaster {

	enum class OpenMode : unsigned int;
	constexpr unsigned int map_to_std(const OpenMode& open_mode);

	enum class OpenMode : unsigned int { Read, Write, AtEnd, Binary, Truncate, Append };

	constexpr OpenMode operator|(const OpenMode& current, const OpenMode& other)
	{
		return static_cast<OpenMode>(map_to_std(current) | map_to_std(other));
	}

	constexpr unsigned int map_to_std(const OpenMode& open_mode)
	{
		switch (open_mode) {
		case OpenMode::Append: {
			return std::ios::app;
		};
		case OpenMode::AtEnd: {
			return std::ios::ate;
		};
		case OpenMode::Binary: {
			return std::ios::binary;
		};
		case OpenMode::Read: {
			return std::ios::in;
		};
		case OpenMode::Write: {
			return std::ios::out;
		};
		case OpenMode::Truncate: {
			return std::ios::trunc;
		};
		default: {
			return 0;
		}
		};
	}
} // namespace Alabaster