#include "platform/Vulkan/CreateInfoStructures.hpp"

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(PlatformTests, VulkanCreateInfos)
{
	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
};

// Demonstrate some basic assertions.
TEST(PlatformTests, VulkanCreateInfosShader)
{
	std::string code("a");
	auto ci = Alabaster::Vulkan::Shader::module(code);

	EXPECT_EQ(ci.codeSize, code.size());
	EXPECT_EQ(ci.sType, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
}
