#pragma once

#include "component/ScriptEntity.hpp"
#include "engine/ScriptingEngine.hpp"
#include "entity/Entity.hpp"

#include <AssetManagerForward.hpp>
#include <SceneForward.hpp>
#include <memory>
#include <uuid.h>

namespace pybind11 {
	class scoped_interpreter;
}

namespace Scripting {

	enum class ScriptMethod : std::uint8_t { Constructor, OnCreate, OnUpdate, OnDelete };

	/// @brief Class for scripting binding to CPP
	class ScriptableEntity {
	public:
		explicit ScriptableEntity(std::string_view, SceneSystem::Entity);

		void on_create();
		void on_update(float ts);
		void on_delete();

		[[nodiscard]] const std::string_view& name() const { return script_name; }

	private:
		void call_method(ScriptMethod method_name, void* parameters = nullptr, std::size_t parameter_size = 0);

		std::string_view script_name;
		SceneSystem::Entity entity;
	};

	struct IEntityUtility {
		virtual ~IEntityUtility() = default;
		virtual std::string_view script_name(SceneSystem::Entity) const = 0;
		virtual uuids::uuid id(SceneSystem::Entity) const = 0;
	};

	struct EntityUtility : public IEntityUtility {
		virtual ~EntityUtility() override = default;
		std::string_view script_name(SceneSystem::Entity) const override;
		uuids::uuid id(SceneSystem::Entity) const override;
	};

	class ScriptEngine {
		using EntityUUIDMap = std::unordered_map<uuids::uuid, ScriptableEntity>;

	public:
		~ScriptEngine();
		ScriptEngine();
		explicit ScriptEngine(SceneSystem::Scene*, std::unique_ptr<IEntityUtility> utility);

		void set_scene(SceneSystem::Scene*);
		[[nodiscard]] const SceneSystem::Scene* get_scene() const;

		template <class... Args> void entity_on_create(Args&&... entity_construction)
		{
			SceneSystem::Entity entity { std::forward<Args>(entity_construction)... };
			valid_scene();
			const auto entity_script_name = entity_utility->script_name(entity);
			const auto id = entity_utility->id(entity);
			if (std::empty(entity_script_name))
				return;
			if (auto&& [k, v] = entity_map.try_emplace(id, std::move(entity_script_name), entity); !v)
				return;
			entity_map.at(id).on_create();
		}
		void entity_on_update(SceneSystem::Entity entity, float ts);
		void entity_on_delete(SceneSystem::Entity entity);

		void register_file_watcher(AssetManager::FileWatcher& watcher);

		static pybind11::scoped_interpreter& get_script_interpreter();

		void set_entity_utility(std::unique_ptr<IEntityUtility> utility) { entity_utility.swap(utility); }
		auto entities() const { return entity_map.size(); }

	private:
		void valid_scene();

		SceneSystem::Scene* current_scene;
		EntityUUIDMap entity_map;

		std::unique_ptr<IEntityUtility> entity_utility;
	};

} // namespace Scripting
