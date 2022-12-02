#pragma once

#include "component/Component.hpp"
#include "entity/Entity.hpp"
#include "serialisation/JsonSubcomponentSerialiser.hpp"
#include "uuid.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace SceneSystem {

	template <Component::IsComponent T> struct serialise_component {
		void operator()(Entity&, nlohmann::json&) {};
	};

} // namespace SceneSystem
