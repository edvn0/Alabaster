#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace Alabaster {

	typedef unsigned char byte;

	struct Buffer {
		Buffer()
			: data(nullptr)
			, size(0)
		{
		}

		Buffer(void* in_data, std::uint32_t in_size)
			: data(in_data)
			, size(in_size)
		{
		}

		static Buffer copy(const void* in_data, std::uint32_t in_size)
		{
			Buffer buffer;
			buffer.allocate(in_size);
			std::memcpy(buffer.data, in_data, in_size);
			return buffer;
		}

		static Buffer copy(const void* in_data, std::size_t in_size)
		{
			Buffer buffer;
			buffer.allocate(static_cast<std::uint32_t>(in_size));
			std::memcpy(buffer.data, in_data, static_cast<std::uint32_t>(in_size));
			return buffer;
		}

		void allocate(std::uint32_t in_size)
		{
			delete[] static_cast<byte*>(this->data);
			this->data = nullptr;

			if (in_size == 0)
				return;

			this->data = new byte[in_size];
			this->size = in_size;
		}

		void release()
		{
			delete[] static_cast<byte*>(this->data);
			this->data = nullptr;
			this->size = 0;
		}

		void zero_initialise()
		{
			if (this->data)
				memset(this->data, 0, this->size);
		}

		template <typename T> T& read(std::uint32_t offset = 0) { return *(T*)(static_cast<byte*>(this->data) + offset); }

		byte* read_bytes(std::uint32_t in_size, std::uint32_t offset) const
		{
			// core_assert_bool(offset + in_size <= this->size);
			byte* buffer = new byte[in_size];
			std::memcpy(buffer, static_cast<byte*>(this->data) + offset, in_size);
			return buffer;
		}

		void write(const void* in_data, std::uint32_t in_size, std::uint32_t offset = 0) const
		{
			// core_assert_bool(offset + in_size <= this->size);
			std::memcpy(static_cast<byte*>(this->data) + offset, in_data, in_size);
		}

		operator bool() const { return this->data; }

		byte& operator[](int index) { return static_cast<byte*>(this->data)[index]; }
		byte operator[](int index) const { return static_cast<byte*>(this->data)[index]; }

		template <typename T> T* as() const { return (T*)this->data; }

		[[nodiscard]] inline std::uint32_t get_size() const { return this->size; }

	public:
		void* data;
		std::uint32_t size;
	};

} // namespace Alabaster
