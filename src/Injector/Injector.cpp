#include "Injector.h"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include "../Common.h"
#include "../Client.h"
#include "Commun.h"

PVOID GetLibraryProcAddress(PSTR LibraryName, PSTR ProcName)
{
	return GetProcAddress(GetModuleHandleA(LibraryName), ProcName);
}
typedef struct _CLIENT_ID
{
	PVOID UniqueProcess;
	PVOID UniqueThread;
} CLIENT_ID, * PCLIENT_ID;

typedef _Null_terminated_ CONST CHAR* LPCSTR, * PCSTR;
typedef HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);
typedef BOOL(WINAPI* PDLL_MAIN)(HMODULE, DWORD, PVOID);

#define NT_SUCCESS(x) ((x) >= 0)
typedef struct _MANUAL_INJECT
{
	PVOID ImageBase;
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseRelocation;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
}MANUAL_INJECT, * PMANUAL_INJECT;

typedef NTSTATUS(NTAPI* _RtlCreateUserThread)(HANDLE ProcessHandle, PSECURITY_DESCRIPTOR SecurityDescriptor, BOOLEAN CreateSuspended, ULONG StackZeroBits, PULONG StackReserved, PULONG StackCommit, PVOID StartAddress, PVOID StartParameter, PHANDLE ThreadHandle, PCLIENT_ID ClientID);
typedef NTSTATUS(NTAPI* _RtlAdjustPrivilege)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
typedef NTSTATUS(NTAPI* _NtOpenProcess)(PHANDLE ProcessHandle, ACCESS_MASK AccessMask, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientID);
typedef NTSTATUS(NTAPI* _NtWriteVirtualMemory)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite, PULONG NumberOfBytesWritten);
typedef NTSTATUS(NTAPI* _NtClose)(HANDLE ObjectHandle);

BOOL executeTls(PMANUAL_INJECT manualInject)
{
	BYTE* codeBase = (BYTE*)manualInject->ImageBase;

	PIMAGE_TLS_DIRECTORY tls;
	PIMAGE_TLS_CALLBACK* callback;

	PIMAGE_DATA_DIRECTORY directory = &(manualInject)->NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
	if (directory->VirtualAddress == 0)
		return TRUE;

	tls = (PIMAGE_TLS_DIRECTORY)(codeBase + directory->VirtualAddress);
	callback = (PIMAGE_TLS_CALLBACK*)tls->AddressOfCallBacks;
	if (callback)
	{
		while (*callback)
		{
			(*callback)((LPVOID)codeBase, DLL_PROCESS_ATTACH, NULL);
			callback++;
		}
	}
	return TRUE;
}

DWORD WINAPI LoadDll(PVOID p)
{
	PMANUAL_INJECT ManualInject;

	HMODULE hModule;
	DWORD i, Function, count, delta;

	PDWORD ptr;
	PWORD list;

	PIMAGE_BASE_RELOCATION pIBR;
	PIMAGE_IMPORT_DESCRIPTOR pIID;
	PIMAGE_IMPORT_BY_NAME pIBN;
	PIMAGE_THUNK_DATA FirstThunk, OrigFirstThunk;

	PDLL_MAIN EntryPoint;

	ManualInject = (PMANUAL_INJECT)p;

	pIBR = ManualInject->BaseRelocation;
	delta = (DWORD)((LPBYTE)ManualInject->ImageBase - ManualInject->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

	// Relocate the image
	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			list = (PWORD)(pIBR + 1);

			for (i = 0; i < count; i++)
			{
				if (list[i])
				{
					ptr = (PDWORD)((LPBYTE)ManualInject->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}

	pIID = ManualInject->ImportDirectory;

	// Resolve DLL imports
	while (pIID->Characteristics)
	{
		OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->OriginalFirstThunk);
		FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->FirstThunk);

		hModule = ManualInject->fnLoadLibraryA((LPCSTR)ManualInject->ImageBase + pIID->Name);

		if (!hModule)
		{
			return FALSE;
		}

		while (OrigFirstThunk->u1.AddressOfData)
		{
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				// Import by ordinal

				Function = (DWORD)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			else
			{
				// Import by name

				pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)ManualInject->ImageBase + OrigFirstThunk->u1.AddressOfData);
				Function = (DWORD)ManualInject->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			OrigFirstThunk++;
			FirstThunk++;
		}

		pIID++;
	}

	if (!executeTls(ManualInject))
		MessageBoxA(0, _xor_("TLS execution failed!").c_str(), 0, MB_ICONERROR | MB_OK);

	if (ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		EntryPoint = (PDLL_MAIN)((LPBYTE)ManualInject->ImageBase + ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
		return EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
	}

	return TRUE;
}

DWORD WINAPI LoadDllEnd()
{
	return 0;
}



void injector::MMInject() {
	auto ProcessName = L"csgo.exe";

	MANUAL_INJECT ManualInject;

	_RtlCreateUserThread RtlCreateUserThread = (_RtlCreateUserThread)GetLibraryProcAddress(_xor_("ntdll.dll").c_str(), _xor_("RtlCreateUserThread").c_str());
	_RtlAdjustPrivilege RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetLibraryProcAddress(_xor_("ntdll.dll").c_str(), _xor_("RtlAdjustPrivilege").c_str());
	_NtOpenProcess NtOpenProcess = (_NtOpenProcess)GetLibraryProcAddress(_xor_("ntdll.dll").c_str(), _xor_("NtOpenProcess").c_str());
	_NtWriteVirtualMemory NtWriteVirtualMemory = (_NtWriteVirtualMemory)GetLibraryProcAddress(_xor_("ntdll.dll").c_str(), _xor_("NtWriteVirtualMemory").c_str());
	_NtClose NtClose = (_NtClose)GetLibraryProcAddress(_xor_("ntdll.dll").c_str(), _xor_("NtClose").c_str());

	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;
	PIMAGE_SECTION_HEADER pISH;

	NTSTATUS status;
	OBJECT_ATTRIBUTES objAttr;
	CLIENT_ID cID;
	DWORD procID = 0;
	BOOLEAN enabled;
	DWORD numBytesWritten;
	HANDLE hThread = INVALID_HANDLE_VALUE;
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	PVOID image, mem;

	RtlAdjustPrivilege(20, TRUE, FALSE, &enabled);
	InitializeObjectAttributes(&objAttr, NULL, 0, NULL, NULL);

	char* enDllData;
	if (!m_Client->recieveDLL(enDllData)) {
		DEBUGLOG("Could't recieve dll!");
		return;
	}

	procID = GetProcessIdFromName(ProcessName);
	cID.UniqueProcess = (PVOID)procID;
	cID.UniqueThread = 0;
	pIDH = (PIMAGE_DOS_HEADER)enDllData;

	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return;
	}

	pINH = (PIMAGE_NT_HEADERS)((LPBYTE)enDllData + pIDH->e_lfanew);
	if (pINH->Signature != IMAGE_NT_SIGNATURE)
	{
		return;
	}

	if (!(pINH->FileHeader.Characteristics & IMAGE_FILE_DLL))
	{
		return;
	}

	if (!NT_SUCCESS(status = NtOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &objAttr, &cID)))
	{
		DEBUGLOG("Unable to open process");
		return;
	}

	image = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!image)
	{
		DEBUGLOG("Unable to allocate memory");
		NtClose(hProcess);
		return;
	}

	if (!NT_SUCCESS(status = NtWriteVirtualMemory(hProcess, image, enDllData, pINH->OptionalHeader.SizeOfHeaders, &numBytesWritten)))
	{
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		NtClose(hProcess);
		return;
	}

	pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);
	for (int i = 0; i < pINH->FileHeader.NumberOfSections; i++)
		NtWriteVirtualMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), (PVOID)((LPBYTE)enDllData + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, &numBytesWritten);

	mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!mem)
	{
		DEBUGLOG("Unable to allocate memory 2");
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		NtClose(hProcess);
		return;
	}

	memset(&ManualInject, 0, sizeof(MANUAL_INJECT));
	ManualInject.ImageBase = image;
	ManualInject.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)image + pIDH->e_lfanew);
	ManualInject.BaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	ManualInject.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	ManualInject.fnLoadLibraryA = LoadLibraryA;
	ManualInject.fnGetProcAddress = GetProcAddress;

	NtWriteVirtualMemory(hProcess, mem, &ManualInject, sizeof(MANUAL_INJECT), &numBytesWritten);
	NtWriteVirtualMemory(hProcess, (PVOID)((PMANUAL_INJECT)mem + 1), LoadDll, (DWORD)LoadDllEnd - (DWORD)LoadDll, &numBytesWritten);

	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((PMANUAL_INJECT)mem + 1), mem, 0, NULL);

	if (!hThread)
	{
		DEBUGLOG("Unable to allocate process thread");
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		NtClose(hProcess);
		return;
	}

	DWORD ExitCode;
	WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, &ExitCode);
	if (!ExitCode)
	{
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		NtClose(hProcess);
		NtClose(hThread);
		return;
	}

	if (pINH->OptionalHeader.AddressOfEntryPoint)
		DEBUGLOG("Injected!");

	NtClose(hThread);
	VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
	NtClose(hProcess);
}


