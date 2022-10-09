#include "av_pch.hpp"

#include "graphics/Shader.hpp"

#include "core/Common.hpp"
#include "graphics/GraphicsContext.hpp"
#include "utilities/FileInputOutput.hpp"

#include <sys/stat.h>

namespace Alabaster {

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
		auto vertex_path = path / ".vert.spv";
		auto fragment_path = path / ".frag.spv";

		verify(std::filesystem::exists(vertex_path));
		verify(std::filesystem::exists(fragment_path));

		vertex_shader_module = create(std::move(IO::read_file(vertex_path)));
		Log::info("Vertex shader read at: {}, and compiled!", vertex_path);
		fragment_shader_module = create(std::move(IO::read_file(fragment_path)));
		Log::info("Fragment shader read at: {}, and compiled!", fragment_path);
	}

} // namespace Alabaster
