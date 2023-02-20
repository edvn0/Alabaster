#pragma once

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Renderer3D.hpp"

extern Alabaster::Application* Alabaster::create(const Alabaster::ApplicationArguments& props);

#include <ProgramOptions.hxx>
#include <any>
#include <filesystem>
#include <memory>
#include <system_error>

int main(int argc, char** argv)
{
	Alabaster::Application* app { nullptr };
	Alabaster::Logger::init();

	const auto root = Alabaster::IO::get_resource_root();

	if (!root) {
		Alabaster::Log::error("Could not determine a suitable root of this project, wherein the folder 'resources' exists.");
		std::exit(1);
	}

	auto cwd = std::filesystem::current_path();
	Alabaster::Log::info("Working directory: {}, root: {}", cwd.string(), (*root).string());

	std::filesystem::path defaults_path = cwd / std::filesystem::path { "resources" } / std::filesystem::path { "cli_defaults.yml" };

	Alabaster::ApplicationArguments props;
	props.width = 4000;
	props.height = 3000;
	props.name = "Alabaster";
	std::string sync_mode = "vsync";
	po::parser parser;
	parser["width"].abbreviation('w').description("The width of the window.").type(po::u32).fallback(std::uint32_t { 1600 }).bind(props.width);
	parser["height"].abbreviation('h').description("The height of the window.").type(po::u32).fallback(std::uint32_t { 900 }).bind(props.height);
	parser["name"].description("Name of the application").type(po::string).fallback(std::string { "Alabaster" }).bind(props.name);
	parser["sync-mode"]
		.abbreviation('m')
		.description("Synchronization. Acceptable values are 'vsync', 'immediate' or 'mailbox'.")
		.type(po::string)
		.fallback(std::string { "vsync" })
		.bind(sync_mode);

	if (!parser(argc, argv)) {
		Alabaster::Log::critical("Could not parse argument options.");
		return 1;
	}

	if (Alabaster::equals_ignore_case(sync_mode, std::string("vsync"))) {
		props.sync_mode = Alabaster::SyncMode::VSync;
	}
	if (Alabaster::equals_ignore_case(sync_mode, std::string("immediate"))) {
		props.sync_mode = Alabaster::SyncMode::Immediate;
	}
	if (Alabaster::equals_ignore_case(sync_mode, std::string("mailbox"))) {
		props.sync_mode = Alabaster::SyncMode::Mailbox;
	}

	Alabaster::Log::info(
		"[EntryPoint] Width: {}, Height: {}, Name: {}, SyncMode: {}", props.width, props.height, props.name, magic_enum::enum_name(props.sync_mode));

	Alabaster::IO::init_with_cwd(*root);

	try {
		app = Alabaster::create(props);
	} catch (const std::system_error& e) {
		Alabaster::Log::error("Error in app creation: {}", e.what());
	}
	AssetManager::ResourceCache::the().initialise();

	try {
		app->run();
	} catch (const std::system_error& e) {
		Alabaster::Log::error("{}", e.what());
	}
	Alabaster::Renderer::shutdown();
	AssetManager::ResourceCache::the().shutdown();
	Alabaster::Allocator::shutdown();

	delete app;

	Alabaster::GraphicsContext::the().destroy();
	Alabaster::Log::critical("Exiting application.");
}
