#include "av_pch.hpp"

#include "graphics/Shader.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "utilities/FileInputOutput.hpp"
#include "vulkan/vulkan_core.h"

#include <platform/Vulkan/CreateInfoStructures.hpp>

namespace Alabaster {

	auto create_default_bindings()
	{
		std::array<VkDescriptorSetLayoutBinding, 2> bindings;
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
		bindings[0].pImmutableSamplers = nullptr; // Optional
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].descriptorCount = 1;
		bindings[1].pImmutableSamplers = nullptr;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		return std::move(bindings);
	}

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

	static VkShaderModule create(const uint32_t* code, size_t size)
	{
		auto create_info = Vulkan::Shader::module(size, code);
		VkShaderModule shader_module;
		vk_check(vkCreateShaderModule(GraphicsContext::the().device(), &create_info, nullptr, &shader_module));

		return shader_module;
	}

	Shader::Shader(const std::string& vertex_path, const std::string& fragment_path)
	{
		auto vertex_shader_module = create(std::move(IO::read_file(vertex_path)));
		auto fragment_shader_module = create(std::move(IO::read_file(fragment_path)));

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
		create_layout();
	}

	Shader::Shader(const std::filesystem::path& p)
		: shader_path(IO::shader(p))
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
		create_layout();
	}

	Shader::Shader(std::vector<uint32_t> vert_spirv, std::vector<uint32_t> frag_spirv)
	{
		auto vertex_shader_module = create(vert_spirv.data(), vert_spirv.size() * sizeof(uint32_t));
		auto fragment_shader_module = create(frag_spirv.data(), frag_spirv.size() * sizeof(uint32_t));

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
		create_layout();
	}

	void Shader::create_layout()
	{
		// TODO: This should obviously be generated from the shader compilation.
		auto bindings = create_default_bindings();

		VkDescriptorSetLayoutCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = bindings.size();
		create_info.pBindings = bindings.data();

		layouts.resize(bindings.size());
		for (uint32_t i = 0; i < bindings.size(); i++) {
			vk_check(vkCreateDescriptorSetLayout(GraphicsContext::the().device(), &create_info, nullptr, &layouts[i]));
		}
	}

	void Shader::destroy()
	{
		for (const auto& stage : shader_stages)
			vkDestroyShaderModule(GraphicsContext::the().device(), stage.module, nullptr);

		for (const auto& layout : layouts) {
			vkDestroyDescriptorSetLayout(GraphicsContext::the().device(), layout, nullptr);
		}

		Log::info("[Shader] Shader stages deleted.");
	}

} // namespace Alabaster
