#pragma once

#include <memory>

class LeakDetector {
public:
	LeakDetector();
	~LeakDetector();

private:
	void report_failure(unsigned int unfreed_bytes);

	/// @brief Should provide some OS specific allocation tracer, on windows I use <crtdbg.h>
	struct impl;

	std::unique_ptr<impl> pimpl;
};
