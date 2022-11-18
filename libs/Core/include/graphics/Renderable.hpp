#pragma once

#include "core/AssetLoader.hpp"
#include "graphics/Material.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Renderer.hpp"

#include <algorithm>
#include <memory>

typedef struct VkCommandBuffer_T* VkCommandBuffer;

namespace Alabaster {

	class Renderable {
	public:
		Renderable(const Mesh& input_mesh, const Material& input_material, std::string name)
		{
			auto& loader = AssetLoader::the();

			auto found_mesh = loader.get_mesh(name);
			if (found_mesh) {
				mesh = *found_mesh;
			} else {
				mesh = loader.create_mesh(name, input_mesh);
			}

			auto found_material = loader.get_material(name);
			if (found_material) {
				material = *found_material;
			} else {
				material = loader.create_material(name, input_material);
			}
		};

		virtual ~Renderable() = default;

		virtual void render(VkCommandBuffer command_buffer) = 0;

	public:
		bool operator<(const Renderable& other) { return material->identifier() < other.material->identifier(); }

	private:
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
		std::string name;

		friend class Renderer;
	};

	class QuadRenderable final : public Renderable {
	public:
		using Renderable::Renderable;
		~QuadRenderable() final = default;

		void render(VkCommandBuffer command_buffer) final { }
	};

} // namespace Alabaster
