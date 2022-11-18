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
		UniformBuffer(VkDeviceSize size, uint32_t binding)
			: UniformBuffer(static_cast<uint32_t>(size), binding) {};

		void set_data(const void* data, uint32_t size, uint32_t offset = 0);
		uint32_t get_binding() const { return binding; }

		const auto& get_buffer() const { return buffer; }

		void destroy();

	private:
		void release();
		void invalidate();

	private:
		VmaAllocation allocation = nullptr;
		VkBuffer buffer = nullptr;
		uint32_t size = 0;
		uint32_t binding = 0;
		bool destroyed { false };
	};

} // namespace Alabaster