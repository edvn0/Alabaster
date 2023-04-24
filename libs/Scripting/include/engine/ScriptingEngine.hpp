#pragma once

#include <memory>

namespace Scripting {

	class ScriptingEngine {
	public:
		ScriptingEngine() = default;
		~ScriptingEngine() = default;

		void initialise();
		void destroy();
	};

} // namespace Scripting