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
}

