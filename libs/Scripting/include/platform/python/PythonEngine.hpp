#pragma once

#include "engine/ScriptingEngine.hpp"

namespace Scripting::Python {

	class PythonEngine : public Scripting::ScriptingEngine {
	public:
		virtual ~PythonEngine() = default;
	};

} // namespace Scripting::Python
