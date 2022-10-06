#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>
using namespace boost::program_options;
#include "core/Application.hpp"

#include <exception>
#include <string>

int main(int ac, char** av)
{
	try {
		// Declare three groups of options.
		options_description general("General options");
		general.add_options()("help", "produce a help message")("help-module", value<std::string>(), "produce a help for a given module")(
			"version", "output the version number");

		options_description gui("GUI options");
		gui.add_options()("display", value<std::string>(), "display to use");

		options_description backend("Backend options");
		backend.add_options()("num-threads", value<int>(), "the initial number of threads");

		// Declare an options description instance which will include
		// all the options
		options_description all("Allowed options");
		all.add(general).add(gui).add(backend);

		// Declare an options description instance which will be shown
		// to the user
		options_description visible("Allowed options");
		visible.add(general).add(gui);

		variables_map vm;
		store(parse_command_line(ac, av, all), vm);

		if (vm.count("help")) {
			std::cout << visible;
			return 0;
		}
		if (vm.count("help-module")) {
			const std::string& s = vm["help-module"].as<std::string>();
			if (s == "gui") {
				std::cout << gui;
			} else if (s == "backend") {
				std::cout << backend;
			} else {
				std::cout << "Unknown module '" << s << "' in the --help-module option\n";
				return 1;
			}
			return 0;
		}
		if (vm.count("num-threads")) {
			std::cout << "The 'num-threads' options was set to " << vm["num-threads"].as<int>() << "\n";
		}
	} catch (std::exception& e) {
		std::cout << e.what() << "\n";
	}

	Alabaster::Application* app = new Alabaster::Application({
		.width = 1280,
		.height = 1080,
		.name = "Alabaster",
	});
	app->run();
}
