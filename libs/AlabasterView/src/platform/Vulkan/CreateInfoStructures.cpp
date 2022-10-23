#include "av_pch.hpp"

#include "platform/Vulkan/CreateInfoStructures.hpp"

namespace Alabaster::Vulkan {
	namespace Shader {
		const VkShaderModuleCreateInfo module(size_t size, const uint32_t* code)
		{
			VkShaderModuleCreateInfo shader_create_info {};
			shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_create_info.codeSize = size;
			shader_create_info.pCode = code;

			return shader_create_info;
		}

		const VkPipelineShaderStageCreateInfo pipeline_stage(ShaderType which, VkShaderModule module, std::string_view entry_point)
		{
			VkPipelineShaderStageCreateInfo vertex_stage {};
			vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertex_stage.stage = static_cast<VkShaderStageFlagBits>(which);
			vertex_stage.module = module;
			vertex_stage.pName = entry_point.data();

			return vertex_stage;
		}
	} // namespace Shader

} // namespace Alabaster::Vulkan