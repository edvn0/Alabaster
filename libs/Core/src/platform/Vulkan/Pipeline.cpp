#include "av_pch.hpp"

#include "graphics/Pipeline.hpp"

#include "core/Common.hpp"
#include "core/Logger.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Shader.hpp"
#include "graphics/VertexBufferLayout.hpp"

#include <vulkan/vulkan.h>

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
			stop();
		}
		}

		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	void Pipeline::invalidate()
	{
#ifdef ALABASTER_MACOS
		spec.line_width = 1.0f;
#endif

		const auto& device = GraphicsContext::the().device();
		const auto& shader = spec.shader;

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext = nullptr;

		const auto& shader_descriptor_layout = shader->descriptor_set_layouts();
		if (non_empty(shader_descriptor_layout)) {
			pipeline_layout_create_info.setLayoutCount = static_cast<std::uint32_t>(shader_descriptor_layout.size());
			pipeline_layout_create_info.pSetLayouts = shader_descriptor_layout.data();
		} else {
			pipeline_layout_create_info.setLayoutCount = static_cast<std::uint32_t>(spec.descriptor_set_layouts.size());
			pipeline_layout_create_info.pSetLayouts = spec.descriptor_set_layouts.data();
		}

		std::vector<VkPushConstantRange> output_ranges;
		if (spec.ranges) {
			const auto& used = *spec.ranges;
			std::uint32_t offset = 0;
			for (const auto& range : used.get_input_ranges()) {
				VkPushConstantRange out {};
				out.offset = offset;
				out.size = range.size;
				out.stageFlags = to_vulkan_flags(range.flags);
				output_ranges.push_back(out);
				offset += range.size;
			}
			pipeline_layout_create_info.pushConstantRangeCount = static_cast<std::uint32_t>(output_ranges.size());
			pipeline_layout_create_info.pPushConstantRanges = output_ranges.data();
		}

		vk_check(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout));

		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.layout = pipeline_layout;
		pipeline_create_info.renderPass = spec.render_pass;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
		input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state.topology = static_cast<VkPrimitiveTopology>(spec.topology);
		input_assembly_state.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterisation_state = {};
		rasterisation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterisation_state.depthClampEnable = VK_FALSE;
		rasterisation_state.rasterizerDiscardEnable = VK_FALSE;
		rasterisation_state.polygonMode = VK_POLYGON_MODE_FILL;

		rasterisation_state.lineWidth = spec.line_width;
		// FIXME: Allow specifying cull mode.
		rasterisation_state.cullMode = spec.backface_culling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT;
		rasterisation_state.cullMode = VK_CULL_MODE_NONE;

		rasterisation_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterisation_state.depthBiasEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment {};
		color_blend_attachment.colorWriteMask
			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.blendEnable = VK_TRUE;

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
		if (spec.topology == Topology::LineList || spec.topology == Topology::LineStrip || spec.wireframe) {
			dynamic_state_enables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
		}

		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.pDynamicStates = dynamic_state_enables.data();
		dynamic_state.dynamicStateCount = static_cast<std::uint32_t>(dynamic_state_enables.size());

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
		depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state.depthTestEnable = spec.depth_test ? VK_TRUE : VK_FALSE;
		depth_stencil_state.depthWriteEnable = spec.depth_write ? VK_TRUE : VK_FALSE;
		depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depth_stencil_state.back.failOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state.back.passOp = VK_STENCIL_OP_KEEP;
		depth_stencil_state.front = depth_stencil_state.back;
		depth_stencil_state.stencilTestEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisample_state = {};
		multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state.sampleShadingEnable = VK_FALSE;

		// Vertex input descriptor
		VertexBufferLayout& vertex_layout = spec.vertex_layout;
		VertexBufferLayout& instance_layout = spec.instance_layout;

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

		std::uint32_t binding = 0;
		std::uint32_t location = 0;
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
		vertex_input_state.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_input_binding_descriptor.size());
		vertex_input_state.pVertexBindingDescriptions = vertex_input_binding_descriptor.data();
		vertex_input_state.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_input_attributes.size());
		vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();

		const auto& stages = shader->stages();

		pipeline_create_info.stageCount = static_cast<std::uint32_t>(stages.size());
		pipeline_create_info.pStages = stages.data();

		pipeline_create_info.pVertexInputState = &vertex_input_state;
		pipeline_create_info.pInputAssemblyState = &input_assembly_state;
		pipeline_create_info.pRasterizationState = &rasterisation_state;
		pipeline_create_info.pColorBlendState = &colour_blend_state;
		pipeline_create_info.pMultisampleState = &multisample_state;
		pipeline_create_info.pViewportState = &viewport_state;
		pipeline_create_info.pDepthStencilState = &depth_stencil_state;
		pipeline_create_info.renderPass = spec.render_pass;
		pipeline_create_info.pDynamicState = &dynamic_state;

		VkPipelineCacheCreateInfo pipeline_cache_info = {};
		pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		{
			auto result = vkCreatePipelineCache(device, &pipeline_cache_info, nullptr, &pipeline_cache);
			if (result != VK_SUCCESS) {
				throw Alabaster::AlabasterException("Could not create pipeline cache.");
			}
		}
		{
			auto result = vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipeline_create_info, nullptr, &pipeline);
			if (result != VK_SUCCESS) {
				throw Alabaster::AlabasterException("Could not create pipeline.");
			}
		}
		Log::info("[Pipeline] Created pipeline with name {}.", spec.debug_name);

		if (spec.shader_owned_by_pipeline) {
			spec.shader->destroy();
		}
	}

	PipelineSpecification& Pipeline::get_specification() { return spec; }
	VkPipelineLayout Pipeline::get_vulkan_pipeline_layout() const { return pipeline_layout; }
	VkPipeline Pipeline::get_vulkan_pipeline() const { return pipeline; }
	const PipelineSpecification& Pipeline::get_specification() const { return spec; }
	bool Pipeline::operator!=(const Pipeline& other) const { return pipeline != other.pipeline; }
	bool Pipeline::operator()(const Pipeline* other) const { return spec.debug_name < other->spec.debug_name; }

	Pipeline::~Pipeline()
	{
		vkDestroyPipelineCache(GraphicsContext::the().device(), pipeline_cache, nullptr);
		vkDestroyPipelineLayout(GraphicsContext::the().device(), pipeline_layout, nullptr);
		vkDestroyPipeline(GraphicsContext::the().device(), pipeline, nullptr);
		Log::info("[Pipeline] Destroyed pipeline {} and its dependents.", spec.debug_name);
	}

} // namespace Alabaster
