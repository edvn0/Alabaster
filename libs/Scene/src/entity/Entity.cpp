#include "scene_pch.hpp"

#include "entity/Entity.hpp"

namespace SceneSystem {

	Entity::~Entity() = default;

	Entity::Entity(Scene* input_scene, entt::entity handle, const std::string&)
		: scene(input_scene)
		, entity_handle(handle)
	{
	}

	Entity::Entity(const Entity& other)
		: scene(other.scene)
		, entity_handle(other.entity_handle == entt::null ? scene->registry.create() : other.entity_handle)
	{
	}

	Entity::Entity(const std::shared_ptr<Scene>& in_scene, const std::string& in_name)
		: Entity(in_scene.get(), entt::null, in_name)
	{
	}

	Entity::Entity(Scene* in_scene, const std::string& in_name)
		: Entity(in_scene, entt::null, in_name)
	{
	}

	Component::Transform& Entity::get_transform()
	{
		if (!scene) {
			return t;
		}

		return get_component<Component::Transform>();
	}

} // namespace SceneSystem
