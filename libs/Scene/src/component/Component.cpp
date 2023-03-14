#include "scene_pch.hpp"

#include "component/Component.hpp"

namespace SceneSystem {

	static uuids::basic_uuid_random_generator uuid_generator(Alabaster::Random::engine());

	Component::ID::ID()
		: identifier(uuid_generator())
	{
	}

	Component::ID::ID(uuids::uuid id)
		: identifier(id)
	{
	}

	std::string Component::ID::to_string() const { return uuids::to_string(identifier); }

	glm::mat4 Component::Transform::to_matrix() const
	{
		return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}

	Component::Mesh::Mesh(const std::shared_ptr<Alabaster::Mesh>& in_mesh)
		: mesh(in_mesh)
	{
	}

	Component::Pipeline::Pipeline(const std::shared_ptr<Alabaster::Pipeline>& in_pipeline)
		: pipeline(in_pipeline)
	{
	}

	Component::Tag::Tag(const std::string& in_tag)
		: tag(in_tag)
	{
	}

} // namespace SceneSystem
