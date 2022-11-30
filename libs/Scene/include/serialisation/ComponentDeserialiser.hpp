#pragma once

#include "component/Component.hpp"
#include "entity/Entity.hpp"
#include "serialisation/JsonSubcomponentSerialiser.hpp"
#include "uuid.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace SceneSystem {

	template <Component::IsComponent T> struct deserialise_component {
		void operator()(nlohmann::json&, Entity&) {};
	};

	template <> struct deserialise_component<Component::ID> {
		void operator()(nlohmann::json& json, Entity& out)
		{
			auto id = json.get<uuids::uuid>();
			out.add_or_set_component<Component::ID>(id);
		}
	};

} // namespace SceneSystem
