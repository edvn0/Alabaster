#pragma once

#include <SceneSystem.hpp>

namespace Scripting::Python {

	class PythonEntity : public SceneSystem::Entity {
	public:
		using SceneSystem::Entity::Entity;

		virtual ~PythonEntity() = default;

		virtual SceneSystem::Component::Transform& get_transform() override;
	};

} // namespace Scripting::Python