#pragma once

#include "core/Application.hpp"
#include "core/Logger.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "utilities/FileInputOutput.hpp"

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

	const auto root = IO::get_resource_root();

	if (!root) {
		Log::error("Could not determine a suitable root of this project, wherein the folder 'resources' exists.");
		std::exit(1);
	}

	auto cwd = std::filesystem::current_path();
	Alabaster::Log::info("Working directory: {}, root: {}", cwd.string(), (*root).string());

	std::filesystem::path defaults_path = cwd / std::filesystem::path { "resources" } / std::filesystem::path { "cli_defaults.yml" };

	Alabaster::CLIOptions desc("Allowed options");
	// clang-format off
	desc.add_options()
		("help", "Show help message")
		("width", boost::program_options::value<uint32_t>()->default_value(1600), "Width of window")
		("height", boost::program_options::value<uint32_t>()->default_value(900), "Height of window")
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

	Alabaster::IO::init_with_cwd(*root);

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
	Alabaster::Renderer::shutdown();
	Alabaster::Allocator::shutdown();

	delete app;

	Alabaster::GraphicsContext::the().destroy();

	Alabaster::Log::critical("Exiting application.");
}
