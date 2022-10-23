#include "av_pch.hpp"

#include "graphics/Shader.hpp"

#include "core/Common.hpp"
#include "graphics/GraphicsContext.hpp"
#include "utilities/FileInputOutput.hpp"

#include <platform/Vulkan/CreateInfoStructures.hpp>

namespace Alabaster {

	std::pair<std::filesystem::path, std::filesystem::path> to_path(const auto& path)
	{
		return { path.string() + ".vert.spv", path.string() + ".frag.spv" };
	}

	static VkShaderModule create(std::string code)
	{
		auto size = code.size();
		auto data = reinterpret_cast<uint32_t*>(code.data());
		auto create_info = Vulkan::Shader::module(size, data);
		VkShaderModule shader_module;
		vk_check(vkCreateShaderModule(GraphicsContext::the().device(), &create_info, nullptr, &shader_module));

		return shader_module;
	}

	Shader::Shader(const std::filesystem::path& p)
		: shader_path(p)
	{
		auto [vert, frag] = to_path(shader_path);
		verify(IO::exists(vert), "Could not find vertex shader.");
		verify(IO::exists(frag), "Could not find fragment shader.");

		auto vertex_shader_module = create(std::move(IO::read_file(vert)));
		auto fragment_shader_module = create(std::move(IO::read_file(frag)));

		VkPipelineShaderStageCreateInfo vertex_stage {};
		vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_stage.module = vertex_shader_module;
		vertex_stage.pName = "main";

		VkPipelineShaderStageCreateInfo fragment_stage {};
		fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_stage.module = fragment_shader_module;
		fragment_stage.pName = "main";

		shader_stages = { vertex_stage, fragment_stage };

		Log::info("[Shader] Shader stages created.");
	}

	void Shader::destroy()
	{
		for (auto& stage : shader_stages) {
			vkDestroyShaderModule(GraphicsContext::the().device(), stage.module, nullptr);
			stage = {};
		}
		Log::info("[Shader] Shader stages deleted.");
	}

} // namespace Alabaster
