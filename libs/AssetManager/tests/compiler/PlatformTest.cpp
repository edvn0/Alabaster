#include "cache/ShaderCache.hpp"
#include "cache/TextureCache.hpp"
#include "graphics/Shader.hpp"
#include "utilities/BitCast.hpp"
#include "utils/LeakDetector.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define EXPECT_HAS_VALUE(x) EXPECT_TRUE(x.has_value())
#define EXPECT_NO_VALUE(x) EXPECT_FALSE(x.has_value())

TEST(AssetManagerTest, NoItemInImageCache)
{
	LeakDetector detector;

	EXPECT_TRUE(true);

	auto* as_u8 = new char[4];
	*as_u8 = '1';
	*(as_u8 + 1) = '2';
	*(as_u8 + 2) = '3';
	*(as_u8 + 3) = '4';

	auto* out = Alabaster::BitCast::reinterpret_as<char*>(as_u8);
	EXPECT_EQ(as_u8, out);

	delete[] as_u8;
};

TEST(AssetManagerTest, NoItemInShaderCache) { EXPECT_TRUE(true); };
