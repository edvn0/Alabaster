#pragma once

namespace Scripting::Registration {

	/// @brief Provide a void operator()() that registers all types for that namespace. Like GLM.
	/// @tparam T the classes
	template <class T> struct RegisterBoundType;

} // namespace Scripting::Registration