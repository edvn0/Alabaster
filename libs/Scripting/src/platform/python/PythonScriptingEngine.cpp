#include "script_pch.hpp"

#include "core/Common.hpp"
#include "engine/ScriptingEngine.hpp"

namespace Scripting {

	void ScriptingEngine::initialise() { Alabaster::Log::info("[PythonScriptingEngine] Initialised!"); }

	void ScriptingEngine::destroy() { Alabaster::Log::info("[PythonScriptingEngine] Destroyed!"); }

} // namespace Scripting
