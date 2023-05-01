#include "utils/LeakDetector.hpp"

#include <crtdbg.h>
#include <gtest/gtest.h>

struct LeakDetector::impl {
	_CrtMemState mem_state;
};

LeakDetector::LeakDetector()
	: pimpl(std::make_unique<impl>())
{
	_CrtMemCheckpoint(&pimpl->mem_state);
}

LeakDetector::~LeakDetector()
{
	_CrtMemState state_now;
	_CrtMemState state_diff;
	_CrtMemCheckpoint(&state_now);
	int diff_result = _CrtMemDifference(&state_diff, &pimpl->mem_state, &state_now);
	if (diff_result)
		report_failure(state_diff.lSizes[1]);
}

void LeakDetector::report_failure(unsigned int unfreed_bytes) { FAIL() << "Memory leak of " << unfreed_bytes << " byte(s) detected."; }
