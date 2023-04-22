#include "platform/python/PythonEntity.hpp"

#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <string>

namespace py = pybind11;
using namespace py::literals;

TEST(InterpreterTests, SomeTest)
{
	py::scoped_interpreter guard {};

	auto locals = py::dict("name"_a = "World", "number"_a = 42);
	py::exec(R"(
        message = "Hello, {name}! The answer is {number}".format(**locals())
    )",
		py::globals(), locals);

	auto message = locals["message"].cast<std::string>();
	ASSERT_EQ(message, "Hello, World! The answer is 42");
}