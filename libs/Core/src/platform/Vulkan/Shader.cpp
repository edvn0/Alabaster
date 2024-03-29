#include "av_pch.hpp"

#include "graphics/Shader.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "utilities/FileInputOutput.hpp"

#include <platform/Vulkan/CreateInfoStructures.hpp>
#include <vulkan/vulkan.h>

namespace Alabaster {

	auto create_default_bindings()
	{
		std::array<VkDescriptorSetLayoutBinding, 3> bindings {};
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		bindings[0].pImmutableSamplers = nullptr; // Optional
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].descriptorCount = 32;
		bindings[1].pImmutableSamplers = nullptr;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

		bindings[2].binding = 2;
		bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[2].descriptorCount = 1;
		bindings[2].pImmutableSamplers = nullptr;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		return bindings;
	}

	std::pair<std::filesystem::path, std::filesystem::path> to_path(const auto& path)
	{
		return { path.string() + ".vert.spv", path.string() + ".frag.spv" };
	}

	static VkShaderModule create(const std::uint32_t* code, std::size_t size)
	{
		const auto create_info = Vulkan::Shader::module(size, code);
		VkShaderModule shader_module;
		vk_check(vkCreateShaderModule(GraphicsContext::the().device(), &create_info, nullptr, &shader_module));

		return shader_module;
	}

	static auto to_spirv(std::string_view path)
	{
		auto data = IO::read_file({ path });
		return std::vector<std::uint32_t> { data.begin(), data.end() };
	}

	Shader::~Shader() = default;

	Shader::Shader(Shader&& s)
		: vertex_stage(std::move(s.vertex_stage))
		, fragment_stage(std::move(s.fragment_stage))
		, shader_path(s.shader_path)
		, layouts(s.layouts)
	{
	}

	Shader::Shader(std::tuple<std::string_view, std::string_view> paths)
		: Shader(std::get<0>(paths), to_spirv(std::get<0>(paths)), to_spirv(std::get<1>(paths)))
	{
	}

	Shader::Shader(const std::string& path_or_name, std::vector<std::uint32_t> vert_spirv, std::vector<std::uint32_t> frag_spirv)
		: vertex_stage(std::make_unique<VkPipelineShaderStageCreateInfo>())
		, fragment_stage(std::make_unique<VkPipelineShaderStageCreateInfo>())
		, shader_path(path_or_name)
	{
		const auto vertex_shader_module = create(vert_spirv.data(), vert_spirv.size() * sizeof(std::uint32_t));
		const auto fragment_shader_module = create(frag_spirv.data(), frag_spirv.size() * sizeof(std::uint32_t));

		*vertex_stage = {};
		vertex_stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_stage->stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_stage->module = vertex_shader_module;
		vertex_stage->pName = "main";

		*fragment_stage = {};
		fragment_stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragment_stage->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_stage->module = fragment_shader_module;
		fragment_stage->pName = "main";

		create_layout();
	}

	const std::array<VkPipelineShaderStageCreateInfo, 2> Shader::stages() const { return { *vertex_stage, *fragment_stage }; }

	void Shader::create_layout()
	{
		// TODO: This should obviously be generated from the shader compilation.
		const auto bindings = create_default_bindings();

		VkDescriptorSetLayoutCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
		create_info.pBindings = bindings.data();

		layouts.resize(bindings.size());
		for (std::uint32_t i = 0; i < bindings.size(); i++) {
			vk_check(vkCreateDescriptorSetLayout(GraphicsContext::the().device(), &create_info, nullptr, &layouts[i]));
		}
	}

	void Shader::destroy()
	{
		vkDestroyShaderModule(GraphicsContext::the().device(), vertex_stage->module, nullptr);
		vkDestroyShaderModule(GraphicsContext::the().device(), fragment_stage->module, nullptr);

		for (const auto& layout : layouts) {
			vkDestroyDescriptorSetLayout(GraphicsContext::the().device(), layout, nullptr);
		}

		Log::info("[Shader] Shader stages for shader {} deleted.", shader_path.string());
	}

	const std::vector<VkDescriptorSetLayout>& Shader::descriptor_set_layouts() const { return layouts; }

} // namespace Alabaster
