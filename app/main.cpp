#include "core/EntryPoint.hpp"

using namespace Alabaster;

class TestApp : public Application {
public:
	TestApp(const ApplicationArguments& args)
		: Application(args) {};
	~TestApp() override = default;
};

Alabaster::Application* Alabaster::create(const Alabaster::ApplicationArguments& props) { return new TestApp(props); }
