#include "engine/ScriptEngine.hpp"

#include <Alabaster.hpp>
#include <SceneSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <string>

struct MockEntityUtility : public Scripting::IEntityUtility {
	virtual ~MockEntityUtility() override = default;
	std::string_view script_name(SceneSystem::Entity) const override { return "empty"; }
	uuids::uuid id(SceneSystem::Entity) const override { return Alabaster::UUID::random_uuid(); }
};

class InterpreterTests : public ::testing::Test {
public:
	InterpreterTests()
		: scene(new SceneSystem::Scene)
		, engine(scene, std::make_unique<MockEntityUtility>())
	{
	}

protected:
	static void SetUpTestSuite() { Alabaster::Logger::init(); }

	static void TearDownTestSuite() { Alabaster::Logger::shutdown(); }

	SceneSystem::Scene* scene;
	Scripting::ScriptEngine engine;
};

TEST_F(InterpreterTests, SceneIsNull)
{
	scene = nullptr;
	engine.set_scene(scene);
	ASSERT_EQ(engine.get_scene(), nullptr);
}

TEST_F(InterpreterTests, SceneIsNotNull) { ASSERT_EQ(engine.get_scene(), scene); }

TEST_F(InterpreterTests, CreateEntity)
{
	auto entity = scene->create_entity("some_name");
	entity.add_component<SceneSystem::Component::ScriptBehaviour>("move");

	auto entity_view = scene->all_with<SceneSystem::Component::ScriptBehaviour>();
	for (const auto e : entity_view)
		engine.entity_on_create(scene, e);

	ASSERT_EQ(engine.entities(), 1);
}

TEST_F(InterpreterTests, CreateMultipleEntities)
{
	static constexpr auto count = 10;
	for (auto i = 0; i < count; i++) {
		auto entity = scene->create_entity("some_name");
		entity.add_component<SceneSystem::Component::ScriptBehaviour>("move");
	}

	auto entity_view = scene->all_with<SceneSystem::Component::ScriptBehaviour>();
	for (const auto e : entity_view)
		engine.entity_on_create(scene, e);

	ASSERT_EQ(engine.entities(), count);
}