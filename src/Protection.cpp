#include "Protection.h"
#include <tlhelp32.h>
using namespace std::chrono_literals;

/* 
	A function that initiates protections by creating two protection 
	threads for protection run loops
*/
void runTimeChecks::setup() {
	std::thread pro(runLoop);
	pro.detach();

	std::thread pro2(runDebugLoop);
	pro2.detach();
}

/* 
	Main protection run loop (run on a seperate thread)
*/
void runTimeChecks::runLoop() {
	while (m_Menu->AppOpen) {
		time = GetTickCount();//store tickcount to measure the time it takes for the function to finish
        antiProcesses();
		std::this_thread::sleep_for(300ms);//sleep to reduce cpu usage
	}
}
