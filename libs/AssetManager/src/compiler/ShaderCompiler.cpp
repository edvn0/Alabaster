#include "am_pch.hpp"

#include "compiler/ShaderCompiler.hpp"

#include "debug_break.h"
#include "utilities/FileInputOutput.hpp"

#include <future>
#include <shaderc/shaderc.hpp>

namespace AssetManager {

	struct ShaderCodeAndName {
		std::string name;
		std::vector<uint32_t> vertex_code;
		std::vector<uint32_t> fragment_code;
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
		for (size_t i = 0; i < a.size() - 1; i++) {
			const auto current = a[i];
			const auto next = a[i + 1];
			if (!true_if_next_is_after_current_function(current, next)) {
				return false;
			}
		}
		return true;
	};

	// Returns GLSL shader source text after preprocessing.
	std::string preprocess_shader(const std::string& source_name, shaderc_shader_kind kind, const std::string& source)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");

		shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			Alabaster::Log::error("[AssetManager] {}", result.GetErrorMessage());
			debug_break();
			return "";
		}

		return { result.cbegin(), result.cend() };
	}

	// Compiles a shader to a SPIR-V binary. Returns the binary as
	// a vector of 32-bit words.
	std::vector<uint32_t> compile_file(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize = false)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		options.SetTargetSpirv(shaderc_spirv_version_1_1);

		const auto pre_process = preprocess_shader(source_name, kind, source);

		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(pre_process, kind, source_name.c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			Alabaster::Log::critical(
				"[VulkanShaderCompiler] Could not compile shader with name: {}. Errors: {}", source_name, module.GetErrorMessage());
			return {};
		}

		return { module.cbegin(), module.cend() };
	}

	ShaderCompiler::ShaderCompiler(const std::filesystem::path& shader_directory)
	{
		std::vector<std::string> all_files_in_shaders = Alabaster::IO::in_directory<std::string>(shader_directory, { ".vert", ".frag" });
		std::sort(all_files_in_shaders.begin(), all_files_in_shaders.end());

		const auto shader_pairs = extract_into_pairs_of_shaders(all_files_in_shaders);

		std::vector<std::future<ShaderCodeAndName>> results;

		Alabaster::assert_that(shader_pairs.size() == all_files_in_shaders.size() / 2);

		for (const auto& [vert, frag] : shader_pairs) {
			Alabaster::assert_that(vert.filename().extension() == ".vert", "Vertex shader file must end with '.vert'.");
			Alabaster::assert_that(frag.filename().extension() == ".frag", "Vertex shader file must end with '.frag'.");

			const auto shader_name = remove_extension<std::filesystem::path>(vert);

			const auto vertex_path = vert;
			const auto fragment_path = frag;

			const auto task = [=]() -> ShaderCodeAndName {
				auto&& [v, f] = compile(shader_name, vertex_path, fragment_path);
				return { shader_name, std::move(v), std::move(f) };
			};

			results.push_back(std::async(std::launch::async, task));
		}

		for (auto& res : results) {
			res.wait();
		}

		for (auto& res : results) {
			auto&& code = res.get();

			shaders.try_emplace(code.name, std::move(code.vertex_code), std::move(code.fragment_code));
		}
	}

	void ShaderCompiler::destroy()
	{
		for (auto& [key, shader] : shaders) {
			shader.destroy();
		}
	}

	std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> ShaderCompiler::compile(
		const std::string& name, const std::filesystem::path& vertex, const std::filesystem::path& fragment)
	{
		const auto read_vertex = Alabaster::IO::read_file(vertex);
		const auto read_fragment = Alabaster::IO::read_file(fragment);

		const auto vertex_spirv = compile_file(name, shaderc_vertex_shader, read_vertex);
		const auto fragment_spirv = compile_file(name, shaderc_fragment_shader, read_fragment);

		return { std::move(vertex_spirv), std::move(fragment_spirv) };
	}

	const Alabaster::Shader& ShaderCompiler::get_by_name(const std::string& shader_name)
	{
		Alabaster::verify(shaders.count(shader_name) != 0, "Futures (shaders) did not contain this shader name");
		return shaders.at(shader_name);
	}

	std::vector<std::pair<std::filesystem::path, std::filesystem::path>> ShaderCompiler::extract_into_pairs_of_shaders(
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

} // namespace AssetManager
