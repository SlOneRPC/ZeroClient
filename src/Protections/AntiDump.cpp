#include "../Protection.h"
#include "Anti-Dumping.h"

/*
	A function to make dumping the app harder
*/
void runTimeChecks::antiDump() {
#ifndef _DEBUG //we only want this to run in release mode
	
	ErasePEHeaderFromMemory();
	SizeOfImage();

#endif // _DEBUG

	timedump = GetTickCount();//store tickcount to measure the time it takes for the function to finish
}

