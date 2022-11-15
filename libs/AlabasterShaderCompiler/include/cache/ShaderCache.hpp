//
// Created by Edwin Carlsson on 2022-11-15.
//

#pragma once

#include "compiler/ShaderCompiler.hpp"

#include <filesystem>

namespace AlabasterShaderCompiler {

	class ShaderCache {
	public:
		ShaderCache(const ShaderCache&) = delete;
		void operator=(const ShaderCache&) = delete;
		ShaderCache(ShaderCache&&) = delete;

		static ShaderCache& the();

		static void initialise();
		static void shutdown();

	public:
		const auto& get_from_cache(const std::string& shader_name) { return compiler.get_by_name(shader_name); }

	private:
		explicit ShaderCache(const std::filesystem::path&);
		~ShaderCache() = default;

	private:
		ShaderCompiler compiler;
	};

} // namespace AlabasterShaderCompiler
