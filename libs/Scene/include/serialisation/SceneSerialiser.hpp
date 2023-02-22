#pragma once

#include "nlohmann/json_fwd.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>

namespace SceneSystem {

	class Scene;
	class Entity;

	class SceneSerialiser {
	public:
		explicit SceneSerialiser(Scene&) noexcept;
		~SceneSerialiser() noexcept
		{
			if (!has_written) {
				write_to_dir();
			}
		};

		void write_to_dir() noexcept;

	private:
		void serialise_to_json();
		nlohmann::json serialise_entity(Entity& entity);

		Scene& scene;
		bool has_written { false };
		nlohmann::json output_json {};
		std::string time_stamp;
	};

} // namespace SceneSystem
