#include "av_pch.hpp"

#include "graphics/Shader.hpp"

#include "core/Common.hpp"
#include "graphics/GraphicsContext.hpp"
#include "utilities/FileInputOutput.hpp"

#include <sys/stat.h>

namespace Alabaster {

	std::pair<std::filesystem::path, std::filesystem::path> to_path(const auto& path)
	{
		return { path.string() + ".vert.spv", path.string() + ".frag.spv" };
	}

	static VkShaderModule create(std::string code)
	{
		VkShaderModuleCreateInfo shader_create_info {};
		shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.codeSize = code.size();
		shader_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shader_module;
		vk_check(vkCreateShaderModule(GraphicsContext::the().device(), &shader_create_info, nullptr, &shader_module));

		return shader_module;
	}

	Shader::Shader(const std::filesystem::path& path)
	{
		auto [vert, frag] = to_path(path);
		verify(std::filesystem::exists(vert), "Could not find vertex shader.");
		verify(std::filesystem::exists(frag), "Could not find fragment shader.");

		vertex_shader_module = create(std::move(IO::read_file(vert)));
		Log::info("Vertex shader read at: {}, and compiled!", vert);
		fragment_shader_module = create(std::move(IO::read_file(frag)));
		Log::info("Fragment shader read at: {}, and compiled!", frag);
	}

} // namespace Alabaster
