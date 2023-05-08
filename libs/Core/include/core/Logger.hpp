#pragma once

#include <filesystem>

// clang-format off
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
// clang-format on

namespace Alabaster {

	enum class LoggerLevel { Debug, Error, Info, Trace, Warn };

	class Logger {
		using LoggerWrapper = std::shared_ptr<spdlog::logger>;

	public:
		static void init(const std::filesystem::path& log_dir);
		static void init();
		static void shutdown();

		static LoggerWrapper get_core_logger();
		static LoggerWrapper get_script_logger();

		static void set_level(LoggerLevel level);
		static void cycle_levels();

	private:
		static inline LoggerWrapper core_logger;
		static inline LoggerWrapper script_logger;

		static inline bool initialised { false };
	};

	namespace Log {

		template <typename... Args> static constexpr auto info(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_core_logger()->info(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto debug(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_core_logger()->debug(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto warn(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_core_logger()->warn(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto error(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_core_logger()->error(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto critical(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_core_logger()->critical(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto trace(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_core_logger()->trace(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

	} // namespace Log

	namespace ScriptLog {

		template <typename... Args> static constexpr auto info(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_script_logger()->info(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto debug(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_script_logger()->debug(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto warn(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_script_logger()->warn(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto error(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_script_logger()->error(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto critical(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_script_logger()->critical(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> static constexpr auto trace(auto&& fmt, Args&&... args) -> void
		{
			::Alabaster::Logger::get_script_logger()->trace(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

	} // namespace ScriptLog

} // namespace Alabaster
