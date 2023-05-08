#pragma once

#include "core/Common.hpp"
#include "core/Logger.hpp"

#include <cstdint>

namespace Alabaster {

	enum class ShaderDataType { None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool };

	static std::uint32_t shader_data_type_size(ShaderDataType type)
	{
		using enum Alabaster::ShaderDataType;

		switch (type) {
		case Float:
			return 4;
		case Float2:
			return 4 * 2;
		case Float3:
			return 4 * 3;
		case Float4:
			return 4 * 4;
		case Mat3:
			return 4 * 3 * 3;
		case Mat4:
			return 4 * 4 * 4;
		case Int:
			return 4;
		case Int2:
			return 4 * 2;
		case Int3:
			return 4 * 3;
		case Int4:
			return 4 * 4;
		case Bool:
			return 1;
		default: {
			Log::error("Unknown ShaderDataType!");
			stop();
		}
		}

		return 0;
	}

	struct VertexBufferElement {
		std::string name;
		ShaderDataType shader_data_type;
		std::uint32_t size;
		std::uint32_t offset { 0 };
		bool normalised { false };

		VertexBufferElement() = default;

		VertexBufferElement(ShaderDataType type, const std::string& input_name, bool is_normalised = false)
			: name(input_name)
			, shader_data_type(type)
			, size(shader_data_type_size(type))
			, normalised(is_normalised)
		{
		}

		std::uint32_t get_component_count() const
		{
			using enum Alabaster::ShaderDataType;

			switch (shader_data_type) {
			case Float:
				return 1;
			case Float2:
				return 2;
			case Float3:
				return 3;
			case Float4:
				return 4;
			case Mat3:
				return 3 * 3;
			case Mat4:
				return 4 * 4;
			case Int:
				return 1;
			case Int2:
				return 2;
			case Int3:
				return 3;
			case Int4:
				return 4;
			case Bool:
				return 1;
			default: {
				Log::error("Never reach here in VertexBuffer.");
				stop();
			}
			}

			return 0;
		}
	};

	class VertexBufferLayout {
	public:
		VertexBufferLayout() = default;

		explicit VertexBufferLayout(const std::initializer_list<VertexBufferElement>& input_elements)
			: elements(input_elements)
		{
			calculate_offsets_and_strides();
		}

		std::uint32_t get_stride() const { return stride; }
		const std::vector<VertexBufferElement>& get_elements() const { return elements; }
		std::uint32_t get_element_count() const { return static_cast<std::uint32_t>(elements.size()); }

		[[nodiscard]] std::vector<VertexBufferElement>::iterator begin() { return elements.begin(); }
		[[nodiscard]] std::vector<VertexBufferElement>::iterator end() { return elements.end(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const { return elements.begin(); }
		[[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const { return elements.end(); }

		inline void log() const
		{
			for (const auto& element : elements) {
				Log::info("Type: {}, Name: {}", enum_name(element.shader_data_type), element.name);
			}
		}

	private:
		void calculate_offsets_and_strides()
		{
			std::uint32_t offset = 0;
			stride = 0;
			for (auto& element : elements) {
				element.offset = offset;
				offset += element.size;
				stride += element.size;
			}
		}

		std::vector<VertexBufferElement> elements;
		std::uint32_t stride = 0;
	};

} // namespace Alabaster
