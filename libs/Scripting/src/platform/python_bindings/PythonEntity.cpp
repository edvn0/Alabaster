#include "script_pch.hpp"

#include "platform/python/PythonEntity.hpp"

#include "entity/Entity.hpp"
#include "platform/python/RegisterGLMTypes.hpp"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace Scripting::Python {

	namespace py = pybind11;
	using namespace SceneSystem;
	using namespace SceneSystem::Component;

	Transform& PythonEntity::get_transform() { PYBIND11_OVERRIDE(SceneSystem::Component::Transform&, ScriptEntity, get_transform, ); }
	Tag& PythonEntity::get_tag() { PYBIND11_OVERRIDE(SceneSystem::Component::Tag&, ScriptEntity, get_tag, ); }

	void PythonEntity::on_create() { PYBIND11_OVERRIDE_PURE(void, ScriptEntity, on_create); }
	void PythonEntity::on_delete() { PYBIND11_OVERRIDE_PURE(void, ScriptEntity, on_delete); }
	void PythonEntity::on_update(const float ts) { PYBIND11_OVERRIDE_PURE(void, ScriptEntity, on_update, ts); }

	void PythonEntity::register_python_module(py::module_& m)
	{
		Registration::PythonGLMBinding { m }();

		py::class_<Transform>(m, "Transform")
			.def(py::init<>())
			.def("to_matrix", &Transform::to_matrix)
			.def_readwrite("position", &Transform::position)
			.def_readwrite("rotation", &Transform::rotation)
			.def_readwrite("scale", &Transform::scale);

		py::class_<Tag>(m, "Tag").def(py::init<>()).def_readwrite("tag", &Tag::tag).def("__repr__", [](const Tag& t) { return t.tag; });

		py::class_<ScriptEntity, PythonEntity>(m, "Entity")
			.def(py::init<>())
			.def("get_transform", &ScriptEntity::get_transform)
			.def("get_tag", &ScriptEntity::get_tag)
			.def("on_update", &ScriptEntity::on_update)
			.def("on_create", &ScriptEntity::on_create)
			.def("on_destroy", &ScriptEntity::on_delete);
	}

	PYBIND11_MODULE(alabaster_entity, m) { PythonEntity::register_python_module(m); }

} // namespace Scripting::Python
