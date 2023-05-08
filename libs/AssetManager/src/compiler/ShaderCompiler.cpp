#include "am_pch.hpp"

#include "compiler/ShaderCompiler.hpp"

#include "compiler/ShaderReflector.hpp"
#include "core/Common.hpp"
#include "utilities/FileInputOutput.hpp"

#include <future>
#include <shaderc/shaderc.hpp>

static constexpr auto should_optimize = false;

namespace AssetManager {

	std::string preprocess_shader(const std::string& source_name, const shaderc_shader_kind kind, const std::string& source)
	{
		const shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");

		const shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			Alabaster::Log::error("[ShaderCompiler] {}", result.GetErrorMessage());
			Alabaster::stop();
			return "";
		}

		return { result.cbegin(), result.cend() };
	}

	std::vector<std::uint32_t> compile_file(
		const std::string& source_name, const shaderc_shader_kind kind, const std::string& source, const bool optimize = false)
	{
		const shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		if constexpr (should_optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		options.SetTargetSpirv(shaderc_spirv_version_1_1);

		const auto pre_process = preprocess_shader(source_name, kind, source);

		const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(pre_process, kind, source_name.c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			throw Alabaster::AlabasterException(
				fmt::format("[ShaderCompiler] Could not compile shader with name: {}. Errors: {}", source_name, module.GetErrorMessage()));
		}

		return { module.cbegin(), module.cend() };
	}

	Alabaster::Shader ShaderCompiler::compile(
		const std::string& name, const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path) const
	{
		const auto&& [vert_spv, frag_spv] = compile_to_spirv(name, vertex_path, fragment_path);

		const auto shader_name = vertex_path.filename();

		ShaderReflector vert_reflector(shader_name.string(), vert_spv);
		ShaderReflector frag_reflector(shader_name.string(), frag_spv);

		return { name, vert_spv, frag_spv };
	}

	std::tuple<std::vector<std::uint32_t>, std::vector<std::uint32_t>> ShaderCompiler::compile_to_spirv(
		const std::string& name, const std::filesystem::path& vertex, const std::filesystem::path& fragment) const
	{
		const auto read_vertex = Alabaster::IO::read_file(vertex);
		const auto read_fragment = Alabaster::IO::read_file(fragment);

		const auto vertex_spv = compile_file(name, shaderc_vertex_shader, read_vertex);
		const auto fragment_spv = compile_file(name, shaderc_fragment_shader, read_fragment);

		return { vertex_spv, fragment_spv };
	}

} // namespace AssetManager
