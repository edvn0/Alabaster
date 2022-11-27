#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define EXPECT_HAS_VALUE(x) EXPECT_TRUE(x.has_value())
#define EXPECT_NO_VALUE(x) EXPECT_FALSE(x.has_value())

TEST(SceneTests, TESTScene1) { EXPECT_TRUE(true); };

TEST(SceneTests, TESTScene2) { EXPECT_TRUE(true); };
