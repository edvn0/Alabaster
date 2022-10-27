#pragma once

#include <memory>

namespace Alabaster {

	class Mesh;
	class Camera;

	class SceneRenderer {
	public:
		SceneRenderer() = default;

		void basic_mesh(const std::unique_ptr<Mesh>& basic_mesh, const std::unique_ptr<Camera>& camera);
	};
} // namespace Alabaster