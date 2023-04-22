#pragma once

#include "core/Buffer.hpp"

#include <memory>

namespace Scripting {

	class ScriptingEngine {
	public:
		virtual ~ScriptingEngine() = default;

		static std::unique_ptr<ScriptingEngine> create();
	};

} // namespace Scripting