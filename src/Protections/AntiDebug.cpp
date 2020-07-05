#include "../Protection.h"
#include "DebuggerDetection.h"
#include "BreakpointDetection.h"
using namespace std::chrono_literals;

/* 
	Anti debuggers run loop (run on a seperate thread)
*/
void runTimeChecks::runDebugLoop() {
	while (m_Menu->AppOpen) {
		antiDebug();
		std::this_thread::sleep_for(300ms);//sleep to reduce cpu usage
	}
}

/* 
   A function that will check for both attached debuggers and breakpoints   
*/
void runTimeChecks::antiDebug() {
#ifndef _DEBUG //we only want this to run in release mode
	
	if (IsDebuggerPresentAPI() || IsDebuggerPresentPEB() || CheckRemoteDebuggerPresentAPI() || NtQueryInformationProcess_ProcessDbgPort()
		|| NtQueryInformationProcess_ProcessDebugFlags() || NtQueryInformationProcess_ProcessDebugObject() || NtGlobalFlag() ||
		IsParentExplorerExe() || ForceFlags() || Int2DCheck() || CloseHandleAPI()) {
		//ban user
		//bluescreen ??
		DEBUGLOG("Debugger attached!");
		m_Menu->AppOpen = false;
	}
	else if (SoftwareBreakpoints(Myfunction_Trap_Debugger, (size_t)(Myfunction_Adresss_Next)-(size_t)(Myfunction_Trap_Debugger)) 
		|| MemoryBreakpoints() || MemoryBreakpoints() || HardwareBreakpoints_GetThreadContext()
		|| HardwareBreakpoints_GetThreadContext() || HardwareBreakpoints_GetThreadContext()) {
		//ban user
		//bluescreen ??
		DEBUGLOG("Breakpoint detected!");
		m_Menu->AppOpen = false;
	}

#endif // _DEBUG

	timedbg = GetTickCount();//store tickcount to measure the time it takes for the function to finish
}