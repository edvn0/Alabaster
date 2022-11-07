#include "av_pch.hpp"

#include "graphics/Shader.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
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

		if (code.empty()) {
			throw AlabasterException("No shader code read.");
		}

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

		// TODO: This should obviously be generated from the shader compilation.
		std::array<VkDescriptorSetLayoutBinding, 1> bindings;
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorSetLayoutCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = bindings.size();
		create_info.pBindings = bindings.data();

		layouts.resize(1);
		vk_check(vkCreateDescriptorSetLayout(GraphicsContext::the().device(), &create_info, nullptr, layouts.data()));
	}

	void Shader::destroy()
	{
		Renderer::free_resource([&shader_stages = shader_stages] {
			for (auto& stage : shader_stages)
				vkDestroyShaderModule(GraphicsContext::the().device(), stage.module, nullptr);

			Log::info("[Shader] Shader stages deleted.");
		});
	}

} // namespace Alabaster
