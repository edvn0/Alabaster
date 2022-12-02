#include "scene_pch.hpp"

#include "entity/Entity.hpp"

namespace SceneSystem {

	Entity::Entity(Scene* scene, entt::entity entity_handle, std::string name)
		: scene(scene)
		, entity_handle(entity_handle == entt::null ? scene->registry.create() : entity_handle)
	{
		if (entity_handle == entt::null) {
			add_component<Component::Tag>(name);
			add_component<Component::ID>();
			add_component<Component::Transform>();
		}
	}

	Entity::Entity(const Entity& other)
		: scene(other.scene)
		, entity_handle(other.entity_handle == entt::null ? scene->registry.create() : other.entity_handle)
	{
		if (entity_handle == entt::null) {
			add_component<Component::Tag>(other.get_component<Component::Tag>().tag);
			add_component<Component::ID>();
			add_component<Component::Transform>();
		}
	}

} // namespace SceneSystem
