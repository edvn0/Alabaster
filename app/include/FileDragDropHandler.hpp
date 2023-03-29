#pragma once

#include <Alabaster.hpp>
#include <SceneSystem.hpp>

namespace App {

	using namespace Alabaster;
	using namespace SceneSystem;

	namespace Filetype {
		enum class Filetypes : std::uint8_t { PNG = 0, TTF, JPEG, JPG, SPV, VERT, FRAG, OBJ, JSON, SCENE };
	}

	template <Filetype::Filetypes Type> struct handle_filetype {
		void operator()(Scene& scene, [[maybe_unused]] const std::filesystem::path& path) const = delete;
	};

	template <> struct handle_filetype<Filetype::Filetypes::PNG> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".png")
				return;

			const TextureProperties props;
			const auto img = Alabaster::Texture::from_filename(path, props);
			auto entity = scene.create_entity("PNG");
			entity.add_component<Component::Texture>(glm::vec4 { 1.0 }, img);
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::SCENE> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".scene")
				return;

			SceneDeserialiser deserialiser(path, scene);
			deserialiser.deserialise();
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::OBJ> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".obj")
				return;

			auto entity = scene.create_entity(path.string());
			entity.add_component<Component::Mesh>(Mesh::from_file(path));
			entity.add_component<Component::Texture>();
			entity.add_component<Component::Pipeline>();
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::TTF> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".ttf")
				return;

			scene.create_entity("TTF");
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::SPV> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".spv")
				return;

			scene.create_entity("SPV");
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::FRAG> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".frag")
				return;

			scene.create_entity("FRAG");
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::VERT> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".vert")
				return;

			scene.create_entity("VERT");
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::JPEG> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".jpeg")
				return;

			const TextureProperties props;
			const auto img = Alabaster::Texture::from_filename(path, props);
			auto entity = scene.create_entity("JPEG");
			entity.add_component<Component::Texture>(glm::vec4 { 1.0 }, img);
		}
	};

	template <> struct handle_filetype<Filetype::Filetypes::JPG> {
		void operator()(Scene& scene, const std::filesystem::path& path) const
		{
			if (path.extension() != ".jpg")
				return;

			const TextureProperties props;
			const auto img = Alabaster::Texture::from_filename(path, props);
			auto entity = scene.create_entity("JPG");
			entity.add_component<Component::Texture>(glm::vec4 { 1.0 }, img);
		}
	};

} // namespace App
