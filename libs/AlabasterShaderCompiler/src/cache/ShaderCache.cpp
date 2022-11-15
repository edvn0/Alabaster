//
// Created by Edwin Carlsson on 2022-11-15.
//
#include "cache/ShaderCache.hpp"

#include "utilities/FileInputOutput.hpp"

namespace AlabasterShaderCompiler {

	static ShaderCache* cache;

	ShaderCache::ShaderCache(const std::filesystem::path& path)
		: compiler(path)
	{
	}

	ShaderCache& ShaderCache::the()
	{
		if (!cache) {
			cache = new ShaderCache(Alabaster::IO::resources() / std::filesystem::path { "shaders" });
		}

		return *cache;
	}

	void ShaderCache::initialise() { the(); }

	void ShaderCache::shutdown()
	{
		cache->compiler.destroy();
		delete cache;
	}

} // namespace AlabasterShaderCompiler
