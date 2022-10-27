#include "av_pch.hpp"

#include "core/Logger.hpp"

#include "core/Common.hpp"

#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Alabaster {

	static LoggerLevel current = LoggerLevel::Info;

	static constexpr auto to_spdlog = [](LoggerLevel level) {
		switch (level) {
		case LoggerLevel::Info:
			return spdlog::level::info;
		case LoggerLevel::Trace:
			return spdlog::level::trace;
		case LoggerLevel::Warn:
			return spdlog::level::warn;
		case LoggerLevel::Error:
			return spdlog::level::err;
		case LoggerLevel::Debug:
			return spdlog::level::debug;
		};
	};

	std::shared_ptr<spdlog::logger> Logger::core_logger;
	std::shared_ptr<spdlog::logger> Logger::client_logger;

	void Logger::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		core_logger = spdlog::stdout_color_mt("Engine");
		core_logger->set_level(spdlog::level::trace);
		client_logger = spdlog::stdout_color_mt("App");
		client_logger->set_level(spdlog::level::trace);

#ifdef ALABASTER_DEBUG
		Logger::set_level(LoggerLevel::Debug);
#elif defined(ALABASTER_RELEASE)
		Logger::set_level(LoggerLevel::Error);
#endif
	}

	void Logger::init(const std::filesystem::path& log_dir)
	{
		std::string logger_dir_engine = (log_dir / "engine.log").string();
		std::string logger_dir_app = (log_dir / "app.log").string();
		spdlog::set_pattern("%^[%T] %n: %v%$");
		core_logger = spdlog::basic_logger_mt("Engine", logger_dir_engine);
		core_logger->set_level(spdlog::level::trace);
		client_logger = spdlog::basic_logger_mt("App", logger_dir_app);
		client_logger->set_level(spdlog::level::trace);
	}

	void Logger::shutdown()
	{
		client_logger.reset();
		core_logger.reset();
		spdlog::drop_all();
	}

	void Logger::cycle_levels()
	{
		switch (current) {
		case LoggerLevel::Debug: {
			current = LoggerLevel::Info;
			break;
		}
		case LoggerLevel::Info: {
			current = LoggerLevel::Trace;
			break;
		}
		case LoggerLevel::Trace: {
			current = LoggerLevel::Warn;
			break;
		}
		case LoggerLevel::Error: {
			current = LoggerLevel::Debug;
			break;
		}
		case LoggerLevel::Warn: {
			current = LoggerLevel::Error;
			break;
		}
		}

		set_level(current);
	}

	void Logger::set_level(LoggerLevel level)
	{
		switch (level) {
		case LoggerLevel::Debug: {
			current = LoggerLevel::Debug;
			break;
		}
		case LoggerLevel::Info: {
			current = LoggerLevel::Info;
			break;
		}
		case LoggerLevel::Trace: {
			current = LoggerLevel::Trace;
			break;
		}
		case LoggerLevel::Error: {
			current = LoggerLevel::Error;
			break;
		}
		case LoggerLevel::Warn: {
			current = LoggerLevel::Warn;
			break;
		}
		}

		Log::critical("[Logger] Current logger level changed to {}", enum_name(current));
		spdlog::level::level_enum spd_level = to_spdlog(current);
		client_logger->set_level(spd_level);
		core_logger->set_level(spd_level);
	}

} // namespace Alabaster
