#include "engine/ScriptEngine.hpp"

#include <SceneSystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <string>

TEST(InterpreterTests, SceneIsNull)
{
	Scripting::ScriptEngine engine;
	engine.set_scene(nullptr);

	ASSERT_EQ(engine.get_scene(), nullptr);
}

TEST(InterpreterTests, SceneIsNotNull)
{
	auto scene = std::make_unique<SceneSystem::Scene>();
	Scripting::ScriptEngine engine;
	engine.set_scene(scene.get());

	ASSERT_EQ(engine.get_scene(), scene.get());
}