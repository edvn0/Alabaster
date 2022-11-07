#pragma once

#include "core/Common.hpp"
#include "graphics/Material.hpp"
#include "graphics/Mesh.hpp"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace Alabaster {

	class AssetLoader {
	private:
		AssetLoader() {};

	public:
		AssetLoader(const AssetLoader&) = delete;
		AssetLoader& operator=(const AssetLoader&) = delete;
		AssetLoader(AssetLoader&&) = delete;
		AssetLoader& operator=(AssetLoader&&) = delete;

		void init()
		{
			global_meshes.reserve(25);
			global_materials.reserve(25);
		}

		static auto& the()
		{
			static AssetLoader test;
			return test;
		}

	public:
		std::optional<std::shared_ptr<Mesh>> get_mesh(const std::string& name)
		{
			if (global_meshes.count(name)) {
				return { global_meshes[name] };
			}
			return {};
		}

		std::optional<std::shared_ptr<Material>> get_material(const std::string& name)
		{
			if (global_materials.count(name)) {
				return { global_materials[name] };
			}
			return {};
		}

		auto create_material(std::string name, const Material& material)
		{
			verify(global_materials.count(name) == 0, "Material already exists.");
			global_materials[name] = std::make_shared<Material>(material);
			return global_materials[name];
		}

		auto create_mesh(std::string name, const Mesh& mesh)
		{
			verify(global_meshes.count(name) == 0, "Mesh already exists.");
			global_meshes[name] = std::make_shared<Mesh>(mesh);
			return global_meshes[name];
		}

	private:
		static inline std::unordered_map<std::string, std::shared_ptr<Mesh>> global_meshes;
		static inline std::unordered_map<std::string, std::shared_ptr<Material>> global_materials;
	};

} // namespace Alabaster
