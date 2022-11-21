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
	const std::uint32_t* code = new std::uint32_t[10];

	const auto size = 10;

	auto ci = Alabaster::Vulkan::Shader::module(size, code);

	EXPECT_EQ(ci.codeSize, size);
	EXPECT_EQ(ci.sType, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	EXPECT_EQ(ci.pCode, code);
}
