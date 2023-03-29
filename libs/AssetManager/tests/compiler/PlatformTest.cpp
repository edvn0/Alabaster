#include "cache/ShaderCache.hpp"
#include "cache/TextureCache.hpp"
#include "graphics/Shader.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define EXPECT_HAS_VALUE(x) EXPECT_TRUE(x.has_value())
#define EXPECT_NO_VALUE(x) EXPECT_FALSE(x.has_value())

TEST(AssetManagerTest, NoItemInImageCache) { EXPECT_TRUE(true); };

TEST(AssetManagerTest, NoItemInShaderCache) { EXPECT_TRUE(true); };
