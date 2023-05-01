#pragma once

#include "component/Component.hpp"
#include "component/ScriptEntity.hpp"
#include "entity/Entity.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace Scripting::Python {

	class PythonEntity : public SceneSystem::ScriptEntity {
	public:
		using SceneSystem::ScriptEntity::ScriptEntity;
		virtual ~PythonEntity() = default;

		SceneSystem::Component::Transform& get_transform() override;
		SceneSystem::Component::Tag& get_tag() override;

		void on_update(const float ts) override;
		void on_create() override;
		void on_delete() override;

		static void register_python_module(pybind11::module_& m);
	};

	class EmbeddedPythonEntity : public SceneSystem::ScriptEntity {
	public:
		using SceneSystem::ScriptEntity::ScriptEntity;
		virtual ~EmbeddedPythonEntity() = default;

		SceneSystem::Component::Transform& get_transform() override;
		SceneSystem::Component::Tag& get_tag() override;

		void on_update(const float ts) override;
		void on_create() override;
		void on_delete() override;
	};

} // namespace Scripting::Python