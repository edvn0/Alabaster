#include "engine/ScriptEngine.hpp"

#include <SceneSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <string>

class InterpreterTests : public ::testing::Test {
public:
	InterpreterTests() { }

protected:
	void SetUp() override { }
	void TearDown() override { }

	Scripting::ScriptEngine engine;
};

TEST_F(InterpreterTests, SceneIsNull)
{
	engine.set_scene(nullptr);

	ASSERT_EQ(engine.get_scene(), nullptr);
}

TEST_F(InterpreterTests, SceneIsNotNull)
{
	auto scene = new SceneSystem::Scene;
	engine.set_scene(scene);

	ASSERT_EQ(engine.get_scene(), scene);
}