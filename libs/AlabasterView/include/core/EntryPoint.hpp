#pragma once

#include "Application.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
#include "Logger.hpp"

extern Alabaster::Application* Alabaster::create(const Alabaster::ApplicationArguments& props);

#include <any>
#include <boost/program_options.hpp>
#include <filesystem>
#include <memory>
#include <system_error>

namespace Alabaster {
	using CLIOptions = boost::program_options::options_description;
	using ArgumentMap = boost::program_options::variables_map;
} // namespace Alabaster

int main(int argc, char** argv)
{
	Alabaster::Application* app { nullptr };
	Alabaster::Logger::init();

	constexpr const auto sanity_checks = [] {
		auto cwd = std::filesystem::current_path();
		const auto app_dir_exists = std::filesystem::exists(cwd / std::filesystem::path { "app" });
		const auto shaders_dir_exists = std::filesystem::exists(
			cwd / std::filesystem::path { "app" } / std::filesystem::path { "resources" } / std::filesystem::path { "shaders" });
		const auto models_dir_exists = std::filesystem::exists(
			cwd / std::filesystem::path { "app" } / std::filesystem::path { "resources" } / std::filesystem::path { "models" });
		const auto textures_dir_exists = std::filesystem::exists(
			cwd / std::filesystem::path { "app" } / std::filesystem::path { "resources" } / std::filesystem::path { "textures" });

		if (!app_dir_exists) {
			Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app' directory there.", cwd.string());
			std::exit(1);
		}

		if (!shaders_dir_exists) {
			Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app/shaders' directory there.", cwd.string());
			std::exit(1);
		}

		if (!models_dir_exists) {
			Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app/models' directory there.", cwd.string());
			std::exit(1);
		}

		if (!textures_dir_exists) {
			Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app/textures' directory there.", cwd.string());
			std::exit(1);
		}
	};

	sanity_checks();

	auto cwd = std::filesystem::current_path();
	Alabaster::Log::info("{}", cwd);

	std::filesystem::path defaults_path = cwd / std::filesystem::path { "resources" } / std::filesystem::path { "cli_defaults.yml" };

	Alabaster::CLIOptions desc("Allowed options");
	// clang-format off
	desc.add_options()
		("help", "Show help message")
		("width", boost::program_options::value<uint32_t>()->default_value(1920), "Width of window")
		("height", boost::program_options::value<uint32_t>()->default_value(1280), "Height of window")
		("name", boost::program_options::value<std::string>()->default_value(std::string { "Alabaster" }), "Title of window")
		("vsync", boost::program_options::value<bool>()->default_value(true), "Window vsync");
	// clang-format on

	Alabaster::ArgumentMap vm;
	try {
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
	} catch (const std::runtime_error& err) {
		Alabaster::Log::error("EntryPoint Error: {}", err.what());
		std::exit(1);
	}

	Alabaster::ApplicationArguments props;

	std::string name = [vm]() {
		try {
			return vm["name"].as<std::string>();
		} catch (const std::bad_any_cast& bad_any_cast) {
			Alabaster::Log::error("Bad cast, Name: {}", bad_any_cast.what());
			std::exit(1);
		}
	}();

	props.name = name.data();

	props.width = [vm]() {
		try {
			return vm["width"].as<uint32_t>();
		} catch (const std::bad_any_cast& bad_any_cast) {
			Alabaster::Log::error("Bad cast, Width: {}", bad_any_cast.what());
			std::exit(1);
		}
	}();

	props.height = [vm]() {
		try {
			return vm["height"].as<uint32_t>();
		} catch (const std::bad_any_cast& bad_any_cast) {
			Alabaster::Log::error("Bad cast, Height: {}", bad_any_cast.what());
			std::exit(1);
		}
	}();

	Alabaster::Log::trace("{}, {}, {}", props.width, props.height, props.name);

	try {
		app = Alabaster::create(props);
	} catch (const std::system_error& e) {
		Alabaster::Log::error("Error in app creation: {}", e.what());
	}

	try {
		app->run();
	} catch (const std::system_error& e) {
		Alabaster::Log::error("{}", e.what());
	}

	delete app;
	Alabaster::GraphicsContext::the().destroy();
	Alabaster::Allocator::shutdown();
}
