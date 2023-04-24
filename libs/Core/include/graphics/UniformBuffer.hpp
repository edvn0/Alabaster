//
// Created by Edwin Carlsson on 2022-10-27.
//

#pragma once

#include "core/Buffer.hpp"

using VkDeviceSize = std::uint64_t;
using VmaAllocation = struct VmaAllocation_T*;
using VkBuffer = struct VkBuffer_T*;

namespace Alabaster {

	class UniformBuffer {
	public:
		UniformBuffer(std::uint32_t size, std::uint32_t input_binding);
		UniformBuffer(VkDeviceSize device_size, std::uint32_t input_binding)
			: UniformBuffer(static_cast<std::uint32_t>(device_size), input_binding) {};

		void set_data(const void* data, const std::uint32_t in_size, const std::uint32_t offset = 0) const;
		std::uint32_t get_binding() const { return binding; }

		const auto& get_buffer() const { return buffer; }

		void destroy();

	private:
		void release();
		void invalidate();

		VmaAllocation allocation { nullptr };
		VkBuffer buffer { nullptr };
		std::uint32_t size { 0 };
		std::uint32_t binding { 0 };
		bool destroyed { false };
	};

} // namespace Alabaster
