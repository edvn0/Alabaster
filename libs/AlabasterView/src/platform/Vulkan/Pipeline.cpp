#include "av_pch.hpp"

#include "graphics/Pipeline.hpp"

#include "core/Common.hpp"
#include "core/Logger.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Shader.hpp"
#include "graphics/VertexBufferLayout.hpp"

namespace Alabaster {

	static VkFormat datatype_to_vulkan(ShaderDataType type)
	{
		switch (type) {
		case ShaderDataType::Float:
			return VK_FORMAT_R32_SFLOAT;
		case ShaderDataType::Float2:
			return VK_FORMAT_R32G32_SFLOAT;
		case ShaderDataType::Float3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case ShaderDataType::Float4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ShaderDataType::Int:
			return VK_FORMAT_R32_SINT;
		case ShaderDataType::Int2:
			return VK_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3:
			return VK_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4:
			return VK_FORMAT_R32G32B32A32_SINT;
		default: {
			Log::error("Unknown shader data type format.");
			debug_break();
		}
		}

		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	void Pipeline::invalidate()
	{
		VkDevice device = GraphicsContext::the().device();
		auto shader = spec.shader;

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext = nullptr;
		// pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		// pipeline_layout_create_info.pSetLayouts = descriptorSetLayouts.data();
		//  pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(vulkanPushConstantRanges.size());
		//  pipeline_layout_create_info.pPushConstantRanges = vulkanPushConstantRanges.data();

		vk_check(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout));

		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.layout = pipeline_layout;
		pipeline_create_info.renderPass = spec.render_pass;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
		input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state.topology = spec.topology;

		VkPipelineRasterizationStateCreateInfo rasterisation_state = {};
		rasterisation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterisation_state.polygonMode = spec.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterisation_state.cullMode = spec.backface_culling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
		rasterisation_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterisation_state.depthClampEnable = VK_FALSE;
		rasterisation_state.rasterizerDiscardEnable = VK_TRUE;
		rasterisation_state.depthBiasEnable = VK_FALSE;
		rasterisation_state.depthBiasConstantFactor = 0.0f; // Optional
		rasterisation_state.depthBiasClamp = 0.0f; // Optional
		rasterisation_state.depthBiasSlopeFactor = 0.0f; // Optional;
		rasterisation_state.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState color_blend_attachment {};
		color_blend_attachment.colorWriteMask
			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colour_blend_state = {};
		colour_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colour_blend_state.logicOpEnable = VK_FALSE;
		colour_blend_state.logicOp = VK_LOGIC_OP_COPY; // Optional
		colour_blend_state.attachmentCount = 1;
		colour_blend_state.pAttachments = &color_blend_attachment;
		colour_blend_state.blendConstants[0] = 0.0f; // Optional
		colour_blend_state.blendConstants[1] = 0.0f; // Optional
		colour_blend_state.blendConstants[2] = 0.0f; // Optional
		colour_blend_state.blendConstants[3] = 0.0f; // Optional

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;

		std::vector<VkDynamicState> dynamic_state_enables;
		dynamic_state_enables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamic_state_enables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		if (spec.topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST || spec.topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP || spec.wireframe)
			dynamic_state_enables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);

		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.pDynamicStates = dynamic_state_enables.data();
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size());

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
		depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state.depthTestEnable = spec.depth_test ? VK_TRUE : VK_FALSE;
		depth_stencil_state.depthWriteEnable = spec.depth_write ? VK_TRUE : VK_FALSE;
		depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state.back.failOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state.back.passOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depth_stencil_state.stencilTestEnable = VK_FALSE;
		depth_stencil_state.front = depth_stencil_state.back;

		// Multi sampling state
		VkPipelineMultisampleStateCreateInfo multisample_state = {};
		multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state.pSampleMask = nullptr;

		// Vertex input descriptor
		VertexBufferLayout& vertex_layout = spec.vertex_layout;
		VertexBufferLayout& instance_layout = spec.instance_layout;

		vertex_layout.log();

		std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptor {};

		VkVertexInputBindingDescription& vertex_input_binding = vertex_input_binding_descriptor.emplace_back();
		vertex_input_binding.binding = 0;
		vertex_input_binding.stride = vertex_layout.get_stride();
		vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		if (non_empty(instance_layout.get_elements())) {
			VkVertexInputBindingDescription& instance_input_binding = vertex_input_binding_descriptor.emplace_back();
			instance_input_binding.binding = 1;
			instance_input_binding.stride = instance_layout.get_stride();
			instance_input_binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		}

		// Input attribute bindings describe shader attribute locations and memory layouts
		std::vector<VkVertexInputAttributeDescription> vertex_input_attributes(
			vertex_layout.get_element_count() + instance_layout.get_element_count());

		uint32_t binding = 0;
		uint32_t location = 0;
		for (const auto& layout : { vertex_layout, instance_layout }) {
			for (const auto& element : layout) {
				auto& attribute = vertex_input_attributes[location];
				attribute.binding = binding;
				attribute.location = location;
				attribute.format = datatype_to_vulkan(element.shader_data_type);
				attribute.offset = element.offset;
				location++;
			}
			binding++;
		}

		// Vertex input state used for pipeline creation
		VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
		vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_input_binding_descriptor.size());
		vertex_input_state.pVertexBindingDescriptions = vertex_input_binding_descriptor.data();
		vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attributes.size());
		vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();

		const auto& stages = shader.stages();

		// Set pipeline shader stage info
		pipeline_create_info.stageCount = static_cast<uint32_t>(stages.size());
		pipeline_create_info.pStages = stages.data();

		// Assign the pipeline states to the pipeline creation info structure
		pipeline_create_info.pVertexInputState = &vertex_input_state;
		pipeline_create_info.pInputAssemblyState = &input_assembly_state;
		pipeline_create_info.pRasterizationState = &rasterisation_state;
		pipeline_create_info.pColorBlendState = &colour_blend_state;
		pipeline_create_info.pMultisampleState = &multisample_state;
		pipeline_create_info.pViewportState = &viewport_state;
		pipeline_create_info.pDepthStencilState = &depth_stencil_state;
		pipeline_create_info.renderPass = spec.render_pass;
		pipeline_create_info.pDynamicState = &dynamic_state;

		// What is this pipeline cache?
		VkPipelineCacheCreateInfo pipeline_cache_info = {};
		pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vk_check(vkCreatePipelineCache(device, &pipeline_cache_info, nullptr, &pipeline_cache));

		// Create rendering pipeline using the specified states
		vk_check(vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipeline_create_info, nullptr, &pipeline));

		Log::info("[Pipeline] Created pipeline with name {}.", spec.debug_name);
	}

	void Pipeline::destroy()
	{
		spec.shader.destroy();
		vkDestroyPipelineCache(GraphicsContext::the().device(), pipeline_cache, nullptr);
		vkDestroyPipelineLayout(GraphicsContext::the().device(), pipeline_layout, nullptr);
		vkDestroyPipeline(GraphicsContext::the().device(), pipeline, nullptr);
		Log::info("[Pipeline] Destroyed pipeline {} and its dependents.", spec.debug_name);
	}

} // namespace Alabaster
