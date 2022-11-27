#include "scene_pch.hpp"

#include "entity/Entity.hpp"

namespace SceneSystem {

	Entity::Entity(Scene& scene, entt::entity entity_handle, std::string name)
		: scene(scene)
		, entity_handle(entity_handle == entt::null ? scene.registry.create() : entity_handle)
	{
		add_component<Component::Tag>(name);
		add_component<Component::ID>();
	};

} // namespace SceneSystem
