#pragma once

#include <filesystem>

// clang-format off
#define YAML_CPP_STATIC_DEFINE
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
// clang-format on

namespace Alabaster {

	class Logger {
		using LoggerWrapper = std::shared_ptr<spdlog::logger>;

	public:
		static void init(const std::filesystem::path& log_dir);
		static void init();
		static void shutdown();

		static LoggerWrapper get_core_logger() { return core_logger; };
		static LoggerWrapper get_client_logger() { return client_logger; };

	private:
		static LoggerWrapper core_logger;
		static LoggerWrapper client_logger;
	};

	namespace Log {

		template <typename... Args> inline static constexpr auto info(auto&& fmt, Args&&... args)
		{
			::Alabaster::Logger::get_core_logger()->info(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> inline static constexpr auto debug(auto&& fmt, Args&&... args)
		{
			::Alabaster::Logger::get_core_logger()->info(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> inline static constexpr auto warn(auto&& fmt, Args&&... args)
		{
			::Alabaster::Logger::get_core_logger()->warn(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> inline static constexpr auto error(auto&& fmt, Args&&... args)
		{
			::Alabaster::Logger::get_core_logger()->error(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

		template <typename... Args> inline static constexpr auto trace(auto&& fmt, Args&&... args)
		{
			::Alabaster::Logger::get_core_logger()->trace(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
		}

	} // namespace Log

} // namespace Alabaster
