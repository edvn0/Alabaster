#include "am_pch.hpp"

#include "compiler/ShaderCompiler.hpp"

#include "debug_break.h"
#include "utilities/FileInputOutput.hpp"

#include <future>
#include <shaderc/shaderc.hpp>

namespace AssetManager {

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

	Alabaster::Shader ShaderCompiler::compile(
		const std::string& name, const std::filesystem::path& vertex_path, const std::filesystem::path& fragment_path) const
	{
		const auto&& [vert_spirv, frag_spirv] = compile_to_spirv(name, vertex_path, fragment_path);
		return Alabaster::Shader(vert_spirv, frag_spirv);
	}

	std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> ShaderCompiler::compile_to_spirv(
		const std::string& name, const std::filesystem::path& vertex, const std::filesystem::path& fragment) const
	{
		const auto read_vertex = Alabaster::IO::read_file(vertex);
		const auto read_fragment = Alabaster::IO::read_file(fragment);

		const auto vertex_spirv = compile_file(name, shaderc_vertex_shader, read_vertex);
		const auto fragment_spirv = compile_file(name, shaderc_fragment_shader, read_fragment);

		return { std::move(vertex_spirv), std::move(fragment_spirv) };
	}

} // namespace AssetManager
