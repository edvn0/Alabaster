//
// Created by Edwin Carlsson on 2022-10-27.
//

#pragma once

#include "core/Buffer.hpp"

#include <vulkan/vulkan.h>
typedef struct VmaAllocation_T* VmaAllocation;

namespace Alabaster {

	class UniformBuffer {
	public:
		UniformBuffer(std::uint32_t size, std::uint32_t binding);
		UniformBuffer(VkDeviceSize size, std::uint32_t binding)
			: UniformBuffer(static_cast<std::uint32_t>(size), binding) {};
		~UniformBuffer();

		void set_data(const void* data, std::uint32_t size, std::uint32_t offset = 0);
		std::uint32_t get_binding() const { return binding; }

		const auto& get_buffer() const { return buffer; }

		void destroy();

	private:
		void release();
		void invalidate();

	private:
		VmaAllocation allocation { nullptr };
		VkBuffer buffer { nullptr };
		std::uint32_t size { 0 };
		std::uint32_t binding { 0 };
		bool destroyed { false };
	};

} // namespace Alabaster
