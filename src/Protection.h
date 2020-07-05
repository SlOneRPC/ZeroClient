#pragma once
#include <cstdint>
#include <Windows.h>
#include <thread>
#include "Menu.h"
#include "Common.h"
namespace runTimeChecks {
	static uint64_t time;
	static uint64_t timedbg;
	static uint64_t timedump;
	void setup();
	void runLoop();
	void runDebugLoop();
	void antiDebug();
	void antiDump();
	void antiProcesses();
}
