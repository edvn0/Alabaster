//
// Created by Edwin Carlsson on 2022-12-12.
//

#pragma once

#include "scene/Scene.hpp"

#include <filesystem>

namespace SceneSystem {

	class SceneDeserialiser {
	public:
		SceneDeserialiser(const std::filesystem::path& in_scene_path, Scene& out)
			: scene(out)
			, scene_path(in_scene_path) {};

		void deserialise();

		Scene& scene;
		std::filesystem::path scene_path;
	};

} // namespace SceneSystem
