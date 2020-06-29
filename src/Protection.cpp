#include "Protection.h"
#include "DebuggerDetection.h"
#include "Anti-Dumping.h"
#include <iostream>
#include "Menu.h"
#ifdef _DEBUG
#define DEBUGLOG(msg) std::cout << msg << std::endl;   
#else
#define DEBUGLOG(msg)
#endif 

void runTimeChecks::setup() {
	std::thread pro(runLoop);
	pro.detach();
}

void runTimeChecks::runLoop() {
	DebugSelf();
	while (true) {
		antiDebug();
		antiDump();
		time = GetTickCount();
	}
}

void runTimeChecks::antiDebug() {
	
	if (IsDebuggerPresentAPI() || IsDebuggerPresentPEB() || CheckRemoteDebuggerPresentAPI() || NtQueryInformationProcess_ProcessDbgPort()
		|| NtQueryInformationProcess_ProcessDebugFlags() || NtQueryInformationProcess_ProcessDebugObject() || NtQueryInformationProcess_SystemKernelDebuggerInformation() 
		|| NtSetInformationThread_ThreadHideFromDebugger() || NtGlobalFlag() || 
		IsParentExplorerExe() || UnhandledExcepFilterTest() || NtQueryObject_ObjectAllTypesInformation()) {
		//ban user
		//bluescreen ??
		//DEBUGLOG("Debugger attached!");
		//m_Menu->AppOpen = false;
	}
	timedbg = GetTickCount();
}

void runTimeChecks::antiDump() {
	ErasePEHeaderFromMemory();
	SizeOfImage();
	timedump = GetTickCount();
}
