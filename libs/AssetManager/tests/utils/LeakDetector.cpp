#include "utils/LeakDetector.hpp"

#include <gtest/gtest.h>

struct LeakDetector::impl { };

LeakDetector::LeakDetector()
	: pimpl(std::make_unique<impl>())
{
}

LeakDetector::~LeakDetector() { }

void LeakDetector::report_failure(unsigned int) { }
