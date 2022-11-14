#include "asc_pch.hpp"

#include "compiler/ShaderCompiler.hpp"

#include "utilities/FileInputOutput.hpp"

#include <shaderc/shaderc.hpp>

namespace AlabasterShaderCompiler {

	// Returns GLSL shader source text after preprocessing.
	std::string preprocess_shader(const std::string& source_name, shaderc_shader_kind kind, const std::string& source)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");

		shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::cerr << result.GetErrorMessage();
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

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::cerr << module.GetErrorMessage();
			return std::vector<uint32_t>();
		}

		return { module.cbegin(), module.cend() };
	}

	ShaderCompiler::ShaderCompiler(const std::filesystem::path& shader_directory)
	{

		std::vector<std::string> all_files_in_shaders = Alabaster::IO::in_directory<std::string>(shader_directory, { ".vert", ".frag" });
		std::sort(all_files_in_shaders.begin(), all_files_in_shaders.end());

		for (size_t i = 0; i < all_files_in_shaders.size(); i += 2) {
			auto fragment = all_files_in_shaders[i];
			auto vertex = all_files_in_shaders[i + 1];
			const auto path = std::filesystem::path { vertex };
			const auto wo_extensions = path.filename().replace_extension().replace_extension().string();
			// shaders.try_emplace(wo_extensions, vertex, fragment);
			auto&& [vert, frag] = compile(wo_extensions, vertex, fragment);

			const auto sh = Alabaster::Shader(std::move(vert), std::move(frag));
			sh.destroy();
		}

		for (auto&& [k, v] : shaders) {
			Alabaster::Log::info("Shader created: \"{}\"", k);
		}
	}

	ShaderCompiler::~ShaderCompiler()
	{
		for (const auto& [key, shader] : shaders) {
			shader.destroy();
		}
	};

	std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> ShaderCompiler::compile(
		const std::string& name, const std::filesystem::path& vertex, const std::filesystem::path& fragment)
	{
		const auto read_vertex = Alabaster::IO::read_file(vertex);
		const auto read_fragment = Alabaster::IO::read_file(fragment);

		const auto vertex_spirv = compile_file(name, shaderc_vertex_shader, read_vertex);
		const auto fragment_spirv = compile_file(name, shaderc_fragment_shader, read_fragment);

		return { std::move(vertex_spirv), std::move(fragment_spirv) };
	}

} // namespace AlabasterShaderCompiler
