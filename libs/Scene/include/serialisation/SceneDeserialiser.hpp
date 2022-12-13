//
// Created by Edwin Carlsson on 2022-12-12.
//

#pragma once

#include "scene/Scene.hpp"

#include <filesystem>

namespace SceneSystem {

	class SceneDeserialiser {
	public:
		SceneDeserialiser() = default;

		void deserialise(const std::filesystem::path& scene_path, Scene& out);
	};

} // namespace SceneSystem
