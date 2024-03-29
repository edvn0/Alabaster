#include "am_pch.hpp"

#include "cache/ShaderCache.hpp"

#include "filesystem/FileSystem.hpp"
#include "utilities/FileInputOutput.hpp"

#include <future>
#include <shaderc/shaderc.hpp>

namespace AssetManager {

	struct ShaderCodeAndName {
		std::string name;
		Alabaster::Shader shader;

		ShaderCodeAndName(const auto& in_name, auto&& in_shader)
			: name(in_name)
			, shader(std::move(in_shader))
		{
		}
	};

	static constexpr auto check_is_sorted = [](auto&& a, auto&& true_if_next_is_after_current_function) -> bool {
		if (a.size() < 1) {
			return true;
		}

		for (std::size_t i = 0; i < a.size() - 1; i++) {
			const auto& current = a[i];
			if (const auto& next = a[i + 1]; !true_if_next_is_after_current_function(current, next)) {
				return false;
			}
		}
		return true;
	};

	void ShaderCache::load_from_directory(const std::filesystem::path& shader_directory)
	{
		using namespace Alabaster::FileSystem;
		const auto all_files_in_shaders = in_directory<std::string>(shader_directory, { ".vert", ".frag" }, true);

		const auto shader_pairs = extract_into_pairs_of_shaders(all_files_in_shaders);

		std::vector<std::future<ShaderCodeAndName>> results;
		Alabaster::assert_that(shader_pairs.size() == all_files_in_shaders.size() / 2);

		const ShaderCompiler compiler;
		for (const auto& [vert, frag] : shader_pairs) {

			const auto& shader_name = remove_extension<std::filesystem::path>(vert);

			const auto& vertex_path = vert;
			const auto& fragment_path = frag;

			auto task
				= [=]() -> ShaderCodeAndName { return ShaderCodeAndName(shader_name, compiler.compile(shader_name, vertex_path, fragment_path)); };

			results.push_back(std::async(std::launch::async, std::move(task)));
		}

		for (auto& res : results) {
			res.wait();
		}

		for (auto& res : results) {

			if (!res.valid()) {
				continue;
			}
			try {
				auto&& code = res.get();
				auto shader = std::make_shared<Alabaster::Shader>(std::move(code.shader));
				shaders.insert(std::make_pair(code.name, shader));
			} catch (const std::exception& e) {
				Alabaster::Log::info("{}", e.what());
			}
		}
	}

	std::vector<std::pair<std::filesystem::path, std::filesystem::path>> ShaderCache::extract_into_pairs_of_shaders(
		const std::vector<std::string>& sorted_shaders_in_directory) const
	{
		Alabaster::assert_that(check_is_sorted(sorted_shaders_in_directory, [](auto&& a, auto&& b) { return b > a; }), "Input vector is not sorted.");

		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> shader_pairs;
		std::unordered_set<std::string> found;

		for (std::size_t i = 0; i < sorted_shaders_in_directory.size(); i++) {
			auto first_shader = sorted_shaders_in_directory[i];
			auto name = remove_extension<std::string>(first_shader, 1);

			if (found.contains(name)) {
				continue;
			}

			for (std::size_t j = i + 1; j < sorted_shaders_in_directory.size(); j++) {

				auto matching_shader = sorted_shaders_in_directory[j];
				auto match = remove_extension<std::string>(matching_shader, 1);

				if (found.contains(match)) {
					continue;
				}

				if (name == match) {
					auto first_shader_filename = std::filesystem::path { first_shader }.filename();
					const bool first_shader_is_vertex_shader = first_shader_filename.extension().string() == "vert";

					if (first_shader_is_vertex_shader) {
						shader_pairs.emplace_back(first_shader, matching_shader);
					} else {
						shader_pairs.emplace_back(matching_shader, first_shader);
					}

					found.emplace(name);
					found.emplace(match);

					break;
				}
			}
		}

		return shader_pairs;
	}

} // namespace AssetManager
