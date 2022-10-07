#include "av_pch.hpp"

#include "core/Logger.hpp"

#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Alabaster {

	std::shared_ptr<spdlog::logger> Logger::core_logger;
	std::shared_ptr<spdlog::logger> Logger::client_logger;

	void Logger::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		core_logger = spdlog::stdout_color_mt("Engine");
		core_logger->set_level(spdlog::level::trace);
		client_logger = spdlog::stdout_color_mt("App");
		client_logger->set_level(spdlog::level::trace);
	}

	void Logger::init(const std::filesystem::path& log_dir)
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		core_logger = spdlog::basic_logger_mt("Engine", log_dir / "engine.log");
		core_logger->set_level(spdlog::level::trace);
		client_logger = spdlog::basic_logger_mt("App", log_dir / "app.log");
		client_logger->set_level(spdlog::level::trace);
	}

	void Logger::shutdown()
	{
		client_logger.reset();
		core_logger.reset();
		spdlog::drop_all();
	}

} // namespace Alabaster