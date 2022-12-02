#include "am_pch.hpp"

#include "compiler/ShaderReflector.hpp"

#include "core/exceptions/AlabasterException.hpp"
#include "core/Logger.hpp"
#include "graphics/PushConstantRange.hpp"
#include "spirv_cross/spirv.hpp"

#include <spirv-cross/spirv_glsl.hpp>
#include <spirv-cross/spirv_reflect.hpp>

namespace AssetManager {

	ShaderReflector::ShaderReflector(std::string shader_file_name, const std::vector<std::uint32_t>& spirv)
		: reflector(std::make_unique<spirv_cross::CompilerGLSL>(spirv))
	{
		// The SPIR-V is now parsed, and we can perform reflection on it.
		spirv_cross::ShaderResources resources = reflector->get_shader_resources();

		// Get all sampled images in the shader.
		for (auto& resource : resources.sampled_images) {
			unsigned set = reflector->get_decoration(resource.id, spv::DecorationDescriptorSet);
			unsigned binding = reflector->get_decoration(resource.id, spv::DecorationBinding);
			Alabaster::Log::info("[ShaderReflector] - Found sampled image for shader {}. Resource name = {},set = {}, binding = {}.",
				shader_file_name, resource.name, set, binding);

			// Modify the decoration to prepare it for GLSL.
			reflector->unset_decoration(resource.id, spv::DecorationDescriptorSet);

			// Some arbitrary remapping if we want.
			reflector->set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
		}

		for (const auto& resource : resources.uniform_buffers) {
			auto active_buffers = reflector->get_active_buffer_ranges(resource.id);

			if (active_buffers.empty())
				break;

			// Discard unused buffers from headers
			const auto& name = resource.name;
			auto& buffer_type = reflector->get_type(resource.base_type_id);
			int member_count = static_cast<std::uint32_t>(buffer_type.member_types.size());
			auto binding = reflector->get_decoration(resource.id, spv::DecorationBinding);
			auto descriptor_set = reflector->get_decoration(resource.id, spv::DecorationDescriptorSet);
			auto size = static_cast<std::uint32_t>(reflector->get_declared_struct_size(buffer_type));

			Alabaster::Log::info("[ShaderReflector] - [Uniform Buffer] {0} ({1}, {2})", name, descriptor_set, binding);
			Alabaster::Log::info("[ShaderReflector] - [Uniform Buffer] Member count: {0}", member_count);
			Alabaster::Log::info("[ShaderReflector] - [Uniform Buffer] Size: {0}", size);
		}

		// Get all sampled images in the shader.
		for (auto& resource : resources.push_constant_buffers) {
			const auto& buffer_name = resource.name;
			auto& buffer_type = reflector->get_type(resource.base_type_id);
			auto buffer_size = static_cast<std::uint32_t>(reflector->get_declared_struct_size(buffer_type));
			auto member_count = static_cast<std::uint32_t>(buffer_type.member_types.size());
			uint32_t buffer_offset = 0;
			if (!reflection_data.push_constant_ranges.empty()) {
				const auto& back = reflection_data.push_constant_ranges.back();
				buffer_offset = back.offset + back.size;
			}

			auto& range = reflection_data.push_constant_ranges.emplace_back();
			range.flags = Alabaster::PushConstantKind::Vertex;
			range.size = buffer_size - buffer_offset;
			range.offset = buffer_offset;

			// buffer.Name = buffer_name;
			// buffer.size = buffer_size - buffer_offset;

			Alabaster::Log::info("[ShaderReflector] - [Push Constant] Name: {0}", buffer_name);
			Alabaster::Log::info("[ShaderReflector] - [Push Constant] Member count: {0}", member_count);
			Alabaster::Log::info("[ShaderReflector] - [Push Constant] Size: {0}", buffer_size);

			for (uint32_t i = 0; i < member_count; i++) {
				auto type = reflector->get_type(buffer_type.member_types[i]);
				const auto& member_name = reflector->get_member_name(buffer_type.self, i);
				auto size = (uint32_t)reflector->get_declared_struct_member_size(buffer_type, i);
				auto offset = reflector->type_struct_member_offset(buffer_type, i) - buffer_offset;

				std::string uniform_name = fmt::format("{}.{}", buffer_name, member_name);

				Alabaster::Log::info("[ShaderReflector] - [Push Constant] - [Member] {}, size: {}, offset: {}", uniform_name, size, offset);
			}
		}

		// Set some options.
		spirv_cross::CompilerGLSL::Options options;
		options.version = 460;
		options.vulkan_semantics = true;
		reflector->set_common_options(options);
	}

} // namespace AssetManager
