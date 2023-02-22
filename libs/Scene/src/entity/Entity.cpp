#include "scene_pch.hpp"

#include "entity/Entity.hpp"

namespace SceneSystem {

	Entity::Entity(Scene* input_scene, entt::entity handle, std::string name)
		: scene(input_scene)
		, entity_handle(handle == entt::null ? scene->registry.create() : handle)
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

	Entity::Entity(const std::unique_ptr<Scene>& in_scene, std::string in_name)
		: Entity(in_scene.get(), entt ::null, in_name) {}

	Entity::Entity(const std::shared_ptr<Scene>& in_scene, std::string in_name)
		: Entity(in_scene.get(), entt ::null, in_name) {}

	Entity::Entity(Scene* in_scene, std::string in_name)
		: Entity(in_scene, entt ::null, in_name) {}
        
} // namespace SceneSystem
