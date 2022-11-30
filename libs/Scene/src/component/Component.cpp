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

	Component::Mesh::Mesh(const std::unique_ptr<Alabaster::Mesh>& mesh)
		: mesh(mesh)
	{
	}

	Component::Pipeline::Pipeline(const std::unique_ptr<Alabaster::Pipeline>& pipeline)
		: pipeline(pipeline)
	{
	}

	Component::Texture::Texture(glm::vec4 col)
		: colour(std::move(col))
	{
	}

	Component::Tag::Tag(const std::string& tag)
		: tag(tag)
	{
	}

} // namespace SceneSystem
