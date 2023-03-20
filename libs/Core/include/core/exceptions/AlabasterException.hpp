#pragma once

#include <stdexcept>
#include <string>

namespace Alabaster {

	class AlabasterException : public std::runtime_error {
	public:
		explicit AlabasterException(const std::string& ex = "") noexcept
			: std::runtime_error(ex)
		{
		}

		explicit AlabasterException(const std::runtime_error& ex) noexcept
			: std::runtime_error(ex)
		{
		}
	};

} // namespace Alabaster
