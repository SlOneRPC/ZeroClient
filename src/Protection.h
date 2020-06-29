#pragma once
#include <cstdint>
#include <Windows.h>
#include <thread>
namespace runTimeChecks {
	static uint64_t time;
	static uint64_t timedbg;
	static uint64_t timedump;
	void setup();
	void runLoop();
	void antiDebug();
	void antiDump();
}
