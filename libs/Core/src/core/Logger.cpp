#include "av_pch.hpp"

#include "core/Logger.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"

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
		default:
			throw AlabasterException("to_spdlog throw, value of level: {}", enum_name(level));
		};
	};

	void Logger::init()
	{
		initialised = true;

		spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l: %v");
		core_logger = spdlog::stdout_color_mt("Engine");
		core_logger->set_level(spdlog::level::trace);
		script_logger = spdlog::stdout_color_mt("Script");
		script_logger->set_level(spdlog::level::trace);

		Logger::set_level(LoggerLevel::Debug);

		initialised = true;
		Log::critical("[Logger] Initialised.");
		ScriptLog::critical("[Logger] Initialised.");
	}

	void Logger::init(const std::filesystem::path& log_dir)
	{

		std::string logger_dir_engine = (log_dir / "engine.log").string();
		std::string logger_dir_app = (log_dir / "app.log").string();
		spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
		core_logger = spdlog::basic_logger_mt("Engine", logger_dir_engine);
		core_logger->set_level(spdlog::level::trace);
		script_logger = spdlog::basic_logger_mt("Script", logger_dir_app);
		script_logger->set_level(spdlog::level::trace);

		initialised = true;
		Log::critical("[Logger] Initialised.");
		ScriptLog::critical("[Logger] Initialised.");
	}

	void Logger::shutdown()
	{
		Log::critical("[Logger] Shutdown.");
		ScriptLog::critical("[Logger] Shutdown.");

		script_logger.reset();
		core_logger.reset();
		spdlog::drop_all();
	}

	Logger::LoggerWrapper Logger::get_core_logger()
	{
		Alabaster::verify(initialised, "Logger is not initialised.");
		return core_logger;
	}

	Logger::LoggerWrapper Logger::get_script_logger()
	{
		Alabaster::verify(initialised, "Logger is not initialised.");
		return script_logger;
	};

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

		spdlog::level::level_enum spd_level = to_spdlog(current);
		script_logger->set_level(spd_level);
		core_logger->set_level(spd_level);
	}

} // namespace Alabaster
