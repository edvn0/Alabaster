#pragma once

#include <fmt/format.h>
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

		template <typename Format, typename... Args>
		explicit AlabasterException(Format&& fmt, Args&&... args) noexcept
			: std::runtime_error(std::string {
				fmt::vformat(static_cast<fmt::string_view>(std::forward<Format>(fmt)), fmt::make_format_args(std::forward<Args>(args)...)) })
		{
		}
	};

} // namespace Alabaster
