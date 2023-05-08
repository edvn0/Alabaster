#pragma once

#include "entity/Entity.hpp"

#include <memory>
#include <string_view>

#ifndef ALABASTER_VISIBILITY
#ifdef ALABASTER_LINUX
#define ALABASTER_VISIBILITY __attribute__((visibility("hidden")))
#else
#define ALABASTER_VISIBILITY
#endif
#endif

namespace Scripting {

	enum class ScriptMethod : std::uint8_t;

	class ScriptingEngine {
	public:
		ScriptingEngine() = default;
		~ScriptingEngine();

		static void call_method(const std::string_view, SceneSystem::Entity, ScriptMethod, void*, std::size_t);

		static void reload_script(std::string_view);

		static void initialise();
		static void destroy();

	private:
		struct ALABASTER_VISIBILITY ScriptEngineData;
		inline static ScriptEngineData* script_data;
	};

} // namespace Scripting
