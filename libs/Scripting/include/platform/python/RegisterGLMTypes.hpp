#pragma once

#include "registration/RegisterBoundType.hpp"

#include <cstdint>
#include <entity/Entity.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <string_view>

#ifndef ALABASTER_VISIBILITY
#ifdef ALABASTER_LINUX
#define ALABASTER_VISIBILITY __attribute__((visibility("hidden")))
#else
#define ALABASTER_VISIBILITY
#endif
#endif

namespace Scripting::Python::GLM {

	namespace py = pybind11;
	using namespace SceneSystem;
	using namespace SceneSystem::Component;

	namespace Types {
		template <class... G> struct glm_items;

		template <> struct glm_items<glm::mat2, glm::mat3, glm::mat4, glm::vec1, glm::vec2, glm::vec3, glm::vec4> { };
		using MathObjects = glm_items<glm::mat2, glm::mat3, glm::mat4, glm::vec1, glm::vec2, glm::vec3, glm::vec4>;
	} // namespace Types

	namespace Names {
		template <class G> constexpr std::string_view glm_name;
		template <> constexpr std::string_view glm_name<glm::mat2> = "Mat2";
		template <> constexpr std::string_view glm_name<glm::mat3> = "Mat3";
		template <> constexpr std::string_view glm_name<glm::mat4> = "Mat4";
		template <> constexpr std::string_view glm_name<glm::vec1> = "Vec1";
		template <> constexpr std::string_view glm_name<glm::vec2> = "Vec2";
		template <> constexpr std::string_view glm_name<glm::vec3> = "Vec3";
		template <> constexpr std::string_view glm_name<glm::vec4> = "Vec4";
	} // namespace Names

	namespace Sizes {
		template <class G> constexpr std::size_t glm_size = 0;
		template <> constexpr std::size_t glm_size<glm::mat2> = 2;
		template <> constexpr std::size_t glm_size<glm::mat3> = 3;
		template <> constexpr std::size_t glm_size<glm::mat4> = 4;
		template <> constexpr std::size_t glm_size<glm::vec1> = 1;
		template <> constexpr std::size_t glm_size<glm::vec2> = 2;
		template <> constexpr std::size_t glm_size<glm::vec3> = 3;
		template <> constexpr std::size_t glm_size<glm::vec4> = 4;
	} // namespace Sizes

	namespace Pybind {
		template <typename... T> static void py_glm(auto& m)
		{
			(
				[&m = m]() {
					py::class_<T>(m, Names::glm_name<T>.data())
						.def(py::init<>())
						.def(py::self + py::self)
						.def(py::self += py::self)
						.def(py::self *= float())
						.def(py::self += float())
						.def(py::self -= float())
						.def(float() * py::self)
						.def(py::self * float())
						.def(-py::self)
						.def("__repr__", &glm::to_string<T>)
						.def("__str__", &glm::to_string<T>)
						.def("__getitem__", [](T& self, unsigned index) {
							Alabaster::verify(index < Sizes::glm_size<T>);
							return self[index];
						});
				}(),
				...);
		}
		template <typename... C> static void register_glm_classes(py::module_& m, Types::glm_items<C...>) { py_glm<C...>(m); }
	} // namespace Pybind

} // namespace Scripting::Python::GLM

namespace Scripting::Registration {

	namespace py = pybind11;
	using namespace SceneSystem;
	using namespace SceneSystem::Component;

	template <> struct ALABASTER_VISIBILITY RegisterBoundType<Python::GLM::Types::MathObjects> {
		py::module_& m;

		void operator()() { Python::GLM::Pybind::register_glm_classes(m, Python::GLM::Types::MathObjects {}); }
	};

	using PythonGLMBinding = RegisterBoundType<Python::GLM::Types::MathObjects>;

} // namespace Scripting::Registration
