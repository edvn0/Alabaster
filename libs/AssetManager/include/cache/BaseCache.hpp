#pragma once

#include "graphics/Image.hpp"
#include "graphics/Shader.hpp"

#include <optional>
#include <string>

namespace AssetManager {

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