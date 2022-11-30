#include "scene_pch.hpp"

#include "serialisation/ComponentDeserialiser.hpp"

namespace SceneSystem {

	template <> struct deserialise_component<Component::ID> {
		void operator()(nlohmann::json& json, Entity& out)
		{
			auto id = json.get<uuids::uuid>();
			out.put_component<Component::ID>(id);
		}
	};

} // namespace SceneSystem
