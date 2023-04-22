#include "script_pch.hpp"

#include "platform/python/PythonEntity.hpp"

#include "platform/python/RegisterGLMTypes.hpp"

#include <entity/Entity.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace Scripting::Python {

	namespace py = pybind11;
	using namespace SceneSystem;
	using namespace SceneSystem::Component;

	Transform& PythonEntity::get_transform() { PYBIND11_OVERRIDE(SceneSystem::Component::Transform&, Entity, get_transform, ); }

	PYBIND11_MODULE(alabaster_entity, m)
	{
		Registration::PythonGLMBinding { m }();

		py::class_<Entity, PythonEntity>(m, "Entity").def(py::init<>()).def("get_transform", &Entity::get_transform);
		py::class_<Transform>(m, "Transform").def(py::init<>()).def("to_matrix", &Transform::to_matrix).def("__repr__", [](const Transform& t) {
			return glm::to_string(t.to_matrix());
		});
	}

} // namespace Scripting::Python
