#include "script_pch.hpp"

#include "core/Common.hpp"
#include "engine/ScriptEngine.hpp"
#include "engine/ScriptingEngine.hpp"
#include "platform/python/PythonEntity.hpp"
#include "platform/python/RegisterGLMTypes.hpp"
#include "utilities/BitCast.hpp"

#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <sstream>
#include <uuid.h>

namespace Scripting {

	struct ALABASTER_VISIBILITY ScriptingEngine::ScriptEngineData {
		std::unordered_map<std::string_view, pybind11::module_> classes {};
		std::unordered_map<uuids::uuid, pybind11::object> instances {};
		std::unordered_map<uuids::uuid, std::shared_ptr<SceneSystem::ScriptEntity>> script_instances {};
	};

	SceneSystem::Component::Transform& Python::EmbeddedPythonEntity::get_transform()
	{
		PYBIND11_OVERRIDE(SceneSystem::Component::Transform&, ScriptEntity, get_transform, );
	}
	SceneSystem::Component::Tag& Python::EmbeddedPythonEntity::get_tag() { PYBIND11_OVERRIDE(SceneSystem::Component::Tag&, ScriptEntity, get_tag, ); }
	void Python::EmbeddedPythonEntity::on_create() { PYBIND11_OVERRIDE_PURE(void, ScriptEntity, on_create); }
	void Python::EmbeddedPythonEntity::on_delete() { PYBIND11_OVERRIDE_PURE(void, ScriptEntity, on_delete); }
	void Python::EmbeddedPythonEntity::on_update(const float ts) { PYBIND11_OVERRIDE_PURE(void, ScriptEntity, on_update, ts); }

	void python_print(pybind11::args args)
	{
		// Concatenate the input arguments into a single string
		std::stringstream ss;
		for (const auto& arg : args)
			ss << pybind11::str(arg).cast<std::string>();

		Alabaster::ScriptLog::info("[Python] {}", ss.str());
	}

	PYBIND11_EMBEDDED_MODULE(alabaster_ecs, m)
	{
		namespace py = pybind11;
		using namespace SceneSystem;
		using namespace SceneSystem::Component;

		Registration::PythonGLMBinding { m }();

		auto builtins = m.import("builtins");
		builtins.def("print", &python_print);

		py::class_<Transform>(m, "Transform")
			.def(py::init<>())
			.def("to_matrix", &Transform::to_matrix)
			.def_readwrite("position", &Transform::position, py::return_value_policy::reference)
			.def_readwrite("rotation", &Transform::rotation, py::return_value_policy::reference)
			.def_readwrite("scale", &Transform::scale, py::return_value_policy::reference)
			.def("__repr__", [](const Transform& t) { return "[Transform] " + glm::to_string(t.to_matrix()); });

		py::class_<Tag>(m, "Tag").def(py::init<>()).def_readwrite("tag", &Tag::tag).def("__repr__", [](const Tag& t) { return "[Tag] " + t.tag; });

		py::class_<entt::entity>(m, "EnttEntity").def("__repr__", [](const entt::entity& e) {
			return std::to_string(static_cast<std::uint32_t>(e));
		});

		py::class_<SceneSystem::Entity>(m, "CppEntity")
			.def(py::init<>())
			.def("get_transform", py::overload_cast<>(&SceneSystem::Entity::get_transform), py::return_value_policy::reference)
			.def("get_entity", &SceneSystem::Entity::get_entity);

		py::class_<ScriptEntity, Python::EmbeddedPythonEntity>(m, "Entity")
			.def(py::init<SceneSystem::Entity>())
			.def(py::init<>())
			.def_readwrite("entity", &ScriptEntity::script_entity)
			.def("get_transform", &ScriptEntity::get_transform, py::return_value_policy::reference)
			.def("get_tag", &ScriptEntity::get_tag, py::return_value_policy::reference)
			.def("on_update", &ScriptEntity::on_update)
			.def("on_create", &ScriptEntity::on_create)
			.def("on_destroy", &ScriptEntity::on_delete);
	}

	namespace {
		template <typename T> static auto script_name(T&& entity)
		{
			return std::forward<T>(entity).template get_component<SceneSystem::Component::ScriptBehaviour>().script_name;
		}

		static constexpr auto valid(auto& scene) { Alabaster::assert_that(scene, "Scene must never be null"); }
	} // namespace

	ScriptingEngine::~ScriptingEngine() = default;

	void ScriptingEngine::reload_script(std::string_view script_name)
	{
		namespace py = pybind11;

		try {
			script_data->classes[script_name].reload();
		} catch (py::error_already_set& e) {
			Alabaster::Log::info("[PythonScriptingEngine] Could not reload module. Reason: {}", e.what());
			return;
		}
	}

	void ScriptingEngine::call_method(const std::string_view script_name, SceneSystem::Entity entity, ScriptMethod method, void* args, std::size_t)
	{
		namespace py = pybind11;
		ScriptEngine::get_script_interpreter();

		// CreateOrGet class
		if (!script_data->classes.contains(script_name)) {
			try {
				script_data->classes[script_name] = py::module_::import(script_name.data());
			} catch (py::error_already_set& e) {
				Alabaster::Log::info("[PythonScriptingEngine] Could not load class. Reason: {}", e.what());
				return;
			}
		}
		auto& script = script_data->classes[script_name];

		// CreateOrGet instance
		auto entity_id = entity.get_component<SceneSystem::Component::ID>().identifier;
		if (!script_data->script_instances.contains(entity_id)) {
			// This is hardcoded for now. The module must supply a behaviour class.
			try {
				script_data->instances[entity_id] = script.attr(py::str { "Behaviour" })(entity);
			} catch (py::error_already_set& e) {
				Alabaster::Log::info("[PythonScriptingEngine] Could not instantiate class. Reason: {}", e.what());
				return;
			}

			script_data->script_instances[entity_id] = std::make_shared<Python::EmbeddedPythonEntity>(entity);
		}

		try {
			// Call method here
			switch (method) {
			case ScriptMethod::OnCreate: {
				script_data->instances[entity_id].attr(py::str { "on_create" })();
				break;
			}
			case ScriptMethod::OnUpdate: {
				auto* float_ts = Alabaster::BitCast::reinterpret_as<float*>(args);
				script_data->instances[entity_id].attr(py::str { "on_update" })(*float_ts);
				break;
			}
			case ScriptMethod::OnDelete: {
				script_data->instances[entity_id].attr(py::str { "on_delete" })();
				script_data->instances.erase(entity_id);
				break;
			}
			default:
				return;
			}
		} catch (py::error_already_set& e) {
			Alabaster::Log::info("[PythonScriptingEngine] Could not call method. Reason: {}", e.what());
		} catch (std::exception& s) {
			Alabaster::Log::info("[PythonScriptingEngine] Cpp exception. Reason: {}", s.what());
		}
	}

	void ScriptingEngine::initialise()
	{
		script_data = new ScriptEngineData();
		Alabaster::Log::info("[PythonScriptingEngine] Initialised!");
	}

	void ScriptingEngine::destroy()
	{
		delete script_data;
		Alabaster::Log::info("[PythonScriptingEngine] Destroyed!");
	}

} // namespace Scripting
