//
// Created by Edwin Carlsson on 2022-10-27.
//

#pragma once

#include "graphics/Buffer.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class UniformBuffer {
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		~UniformBuffer();

		void set_data(const void* data, uint32_t size, uint32_t offset = 0);
		uint32_t get_binding() const { return binding; }

		const VkDescriptorBufferInfo& get_descriptor_buffer_info() const { return descriptor_info; }

	private:
		void release();
		void invalidate();

	private:
		VmaAllocation allocation = nullptr;
		VkBuffer buffer;
		VkDescriptorBufferInfo descriptor_info {};
		uint32_t size = 0;
		uint32_t binding = 0;
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

		uint8_t* local_data { nullptr };
	};

} // namespace Alabaster