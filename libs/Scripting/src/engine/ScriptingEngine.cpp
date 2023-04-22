#include "script_pch.hpp"

#include "engine/ScriptingEngine.hpp"

#include "core/exceptions/AlabasterException.hpp"

#ifdef ALABASTER_HAS_PYTHON
#include "platform/python/PythonEngine.hpp"
#endif

namespace Scripting {

	class NoScriptEngineFoundException : public Alabaster::AlabasterException {
	public:
		using AlabasterException::AlabasterException;
	};

	std::unique_ptr<ScriptingEngine> Scripting::ScriptingEngine::create()
	{
#ifdef ALABASTER_HAS_PYTHON
		return std::make_unique<Python::PythonEngine>();
#else
		throw NoScriptEngineFoundException("Could not find a suitable script engine.");
#endif
	}
} // namespace Scripting
