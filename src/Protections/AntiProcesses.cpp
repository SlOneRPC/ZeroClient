#include "../Protection.h"
#include "Commun.h"

/*
	A function to check for any currently running analysis software
*/
void runTimeChecks::antiProcesses() {

#ifdef _DEBUG //we only want this to run in debug mode
	//we dont want console spam so we need a bool to check if a process has already be detected
	static bool detectedOnce = false;
	if (detectedOnce)
		return;
#endif // _DEBUG

	const std::string maliciousWindows[] =
	{ _xor_(_T("ollydbg.exe")),			// OllyDebug debugger
		_xor_(_T("ProcessHacker.exe")),	    // Process Hacker
		_xor_(_T("tcpview.exe")),			// Part of Sysinternals Suite
		_xor_(_T("autoruns.exe")),			// Part of Sysinternals Suite
		_xor_(_T("autorunsc.exe")),		// Part of Sysinternals Suite
		_xor_(_T("filemon.exe")),			// Part of Sysinternals Suite
		_xor_(_T("procmon.exe")),			// Part of Sysinternals Suite
		_xor_(_T("regmon.exe")),			// Part of Sysinternals Suite
		_xor_(_T("procexp.exe")),			// Part of Sysinternals Suite
		_xor_(_T("idaq.exe")),				// IDA Pro Interactive Disassembler
		_xor_(_T("idaq64.exe")),			// IDA Pro Interactive Disassembler
		_xor_(_T("ImmunityDebugger.exe")), // ImmunityDebugger
		_xor_(_T("Wireshark.exe")),		// Wireshark packet sniffer
		_xor_(_T("dumpcap.exe")),			// Network traffic dump tool
		_xor_(_T("HookExplorer.exe")),		// Find various types of runtime hooks
		_xor_(_T("ImportREC.exe")),		// Import Reconstructor
		_xor_(_T("PETools.exe")),			// PE Tool
		_xor_(_T("LordPE.exe")),			// LordPE
		_xor_(_T("dumpcap.exe")),			// Network traffic dump tool
		_xor_(_T("SysInspector.exe")),		// ESET SysInspector
		_xor_(_T("proc_analyzer.exe")),	// Part of SysAnalyzer iDefense
		_xor_(_T("sysAnalyzer.exe")),		// Part of SysAnalyzer iDefense
		_xor_(_T("sniff_hit.exe")),		// Part of SysAnalyzer iDefense
		_xor_(_T("windbg.exe")),			// Microsoft WinDbg
		_xor_(_T("joeboxcontrol.exe")),	// Part of Joe Sandbox
		_xor_(_T("joeboxserver.exe")),		// Part of Joe Sandbox
		_xor_(_T("fiddler.exe")),
		_xor_(_T("tv_w32.exe")),
		_xor_(_T("tv_x64.exe")),
		_xor_(_T("x64dbg.exe")),			//Part of x64dbg
		_xor_(_T("x32dbg.exe")),			//Part of x64dbg
	};

	for (std::string proc : maliciousWindows) {
		if (GetProcessIdFromName(proc)) {
#ifdef _DEBUG //we only want this to run in debug mode
			
			DEBUGLOG("malicious program open! " << proc.c_str());
			detectedOnce = true;

#else //we only want this to run in release mode

			m_Menu->AppOpen = false;

#endif // _DEBUG
		}
	}

}
