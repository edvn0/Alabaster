#include "am_pch.hpp"

#include "cache/ShaderCache.hpp"

#include "debug_break.h"
#include "utilities/FileInputOutput.hpp"

#include <future>
#include <shaderc/shaderc.hpp>

namespace AssetManager {

	struct ShaderCodeAndName {
		std::string name;
		Alabaster::Shader shader;
	};

	template <typename T>
	static constexpr auto remove_extension = [](const T& path, uint32_t count = 2) {
		if constexpr (std::is_same_v<std::string, T>) {
			auto converted_path = std::filesystem::path { path };
			if (count != 2) {
				auto out = converted_path.filename();
				for (auto i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return converted_path.filename().replace_extension().replace_extension().string();
		} else {
			auto out_converted = std::filesystem::path { path.filename() };
			if (count != 2) {
				auto out = out_converted;
				for (auto i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return out_converted.replace_extension().replace_extension().string();
		}
	};

	static constexpr auto check_is_sorted = [](auto&& a, auto&& true_if_next_is_after_current_function) -> bool {
		if (a.size() < 1) {
			return true;
		}

		for (size_t i = 0; i < a.size() - 1; i++) {
			const auto current = a[i];
			const auto next = a[i + 1];
			if (!true_if_next_is_after_current_function(current, next)) {
				return false;
			}
		}
		return true;
	};

	template <class T> void ShaderCache<T>::load_from_directory(const std::filesystem::path& shader_directory)
	{
		using namespace Alabaster::IO;
		auto all_files_in_shaders = in_directory<std::string>(shader_directory, { ".vert", ".frag" }, true);

		const auto shader_pairs = extract_into_pairs_of_shaders(all_files_in_shaders);

		std::vector<std::future<ShaderCodeAndName>> results;
		Alabaster::assert_that(shader_pairs.size() == all_files_in_shaders.size() / 2);

		ShaderCompiler compiler;
		for (const auto& [vert, frag] : shader_pairs) {

			const auto shader_name = remove_extension<std::filesystem::path>(vert);

			const auto vertex_path = vert;
			const auto fragment_path = frag;

			const auto task = [=]() -> ShaderCodeAndName {
				const Alabaster::Shader shader = compiler.compile(shader_name, vertex_path, fragment_path);
				return { shader_name, std::move(shader) };
			};

			results.push_back(std::async(std::launch::async, task));
		}

		for (auto& res : results) {
			res.wait();
		}

		for (auto& res : results) {
			auto&& code = res.get();

			shaders.try_emplace(code.name, code.shader);
		}
	}

	template <typename T>
	std::vector<std::pair<std::filesystem::path, std::filesystem::path>> ShaderCache<T>::extract_into_pairs_of_shaders(
		const std::vector<std::string>& sorted_shaders_in_directory)
	{
		Alabaster::assert_that(check_is_sorted(sorted_shaders_in_directory, [](auto&& a, auto&& b) { return b > a; }), "Input vector is not sorted.");

		std::vector<std::pair<std::filesystem::path, std::filesystem::path>> shader_pairs;
		std::unordered_set<std::string> found;

		for (size_t i = 0; i < sorted_shaders_in_directory.size(); i++) {
			auto first_shader = sorted_shaders_in_directory[i];
			auto name = remove_extension<std::string>(first_shader, 1);

			if (found.contains(name)) {
				continue;
			}

			for (size_t j = i + 1; j < sorted_shaders_in_directory.size(); j++) {

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

	template class ShaderCache<Alabaster::Shader>;

} // namespace AssetManager
