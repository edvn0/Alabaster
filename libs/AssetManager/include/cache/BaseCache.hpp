#pragma once

#include "graphics/Image.hpp"
#include "graphics/Shader.hpp"

#include <optional>
#include <string>

namespace AssetManager {

	template <typename T>
	static constexpr auto remove_extension = [](const T& path, uint32_t count = 2) {
		if constexpr (std::is_same_v<std::string, T>) {
			auto converted_path = std::filesystem::path { path };
			if (count != 2) {
				auto out = converted_path.filename();
				for (auto i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return converted_path.filename().replace_extension().replace_extension().string();
		} else {
			auto out_converted = std::filesystem::path { path.filename() };
			if (count != 2) {
				auto out = out_converted;
				for (auto i = 0; i < count; i++) {
					out.replace_extension();
				}
				return out.string();
			}
			return out_converted.replace_extension().replace_extension().string();
		}
	};

	template <template <class> class Child, class ItemType> class BaseCache {
	public:
		[[nodiscard]] std::optional<const ItemType*> get_from_cache(const std::string& item_name) { return child().get_from_cache_impl(item_name); }

		[[nodiscard]] bool add_to_cache(const std::string& name, ItemType* input) { return child().add_to_cache_impl(name, input); };

		void destroy() { child().destroy_impl(); }

	private:
		auto& child() { return *static_cast<Child<ItemType>*>(this); }

	private:
		friend Child<ItemType>;
	};

	template <typename T> struct CacheCreateRead {
		virtual ~CacheCreateRead() = default;
		virtual const T* get(const std::string& name, std::unordered_map<std::string, T>& out) = 0;
		virtual void create(const std::string& name, T* data, std::unordered_map<std::string, T>& out) = 0;
	};

	struct DefaultShaderCrud : public CacheCreateRead<Alabaster::Shader> {
		virtual ~DefaultShaderCrud() override = default;

		const Alabaster::Shader* get(const std::string& name, std::unordered_map<std::string, Alabaster::Shader>& out) override
		{
			return &out.at(name);
		};

		void create(const std::string& name, Alabaster::Shader* data, std::unordered_map<std::string, Alabaster::Shader>& out) override
		{
			out.try_emplace(name, *data);
		};
	};

	struct DefaultImageCrud : public CacheCreateRead<Alabaster::Image> {
		virtual ~DefaultImageCrud() override = default;

		const Alabaster::Image* get(const std::string& name, std::unordered_map<std::string, Alabaster::Image>& out) override
		{
			return &out.at(name);
		};

		void create(const std::string& name, Alabaster::Image* data, std::unordered_map<std::string, Alabaster::Image>& out) override
		{
			out.try_emplace(name, *data);
		};
	};

} // namespace AssetManager