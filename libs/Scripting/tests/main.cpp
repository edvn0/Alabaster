#include "core/Common.hpp"

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	Alabaster::Logger::init();
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}