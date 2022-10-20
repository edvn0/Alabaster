#pragma once

#include <stdexcept>

namespace Alabaster {

	class AlabasterException : public std::runtime_error {
	public:
		explicit AlabasterException(const std::string& ex = "") noexcept
			: std::runtime_error("[AlabasterException]: " + ex)
		{
		}
	};

} // namespace Alabaster
