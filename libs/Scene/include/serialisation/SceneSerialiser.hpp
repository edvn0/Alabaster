#pragma once

#include "AssetManager.hpp"

#include <filesystem>
#include <future>
#include <nlohmann/json.hpp>
#include <vector>

namespace SceneSystem {

	class Scene;
	class Entity;

	nlohmann::json serialise_entity(Entity& entity);

	class SceneSerialiser {
	public:
		explicit SceneSerialiser(Scene&) noexcept;

		~SceneSerialiser() noexcept
		{
			if (!has_written) {
				write_to_dir();
			}
		};

	private:
		void write_to_dir() noexcept;
		void serialise_to_json();

		Scene& scene;
		bool has_written { false };
		nlohmann::json output_json {};
		std::vector<std::future<nlohmann::json>> futures;
		std::string time_stamp;
	};

} // namespace SceneSystem
