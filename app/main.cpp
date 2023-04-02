#include "AlabasterLayer.hpp"
#include "core/EntryPoint.hpp"

using namespace Alabaster;

class TestApp : public Application {
public:
	using Application::Application;
	~TestApp() override = default;

	void on_init() override { push_layer<AlabasterLayer>(); }
};

Alabaster::Application* Alabaster::create(const Alabaster::ApplicationArguments& props) { return new TestApp(props); }
