#include "pch.h"
#include "Process.h"

#define ERROR(msg) { /* Error handling needs to be implemented */ }

Process::Process()
{
}

Process::~Process()
{
}

DWORD Process::GetProcessID(LPCTSTR ProcessName)
{
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) 
	{
		do 
		{
			if (!lstrcmpi(pt.szExeFile, ProcessName)) 
			{
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} 
		while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap);
	return 0;
}

HMODULE Process::LoadLibraryRemotely(DWORD ProcessID, const char* LibraryPath)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessID);

	void* location = VirtualAllocEx(hProcess, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if(!location)
		ERROR("Failed to allocate memory in remote process")

	BOOL writeSuccess = WriteProcessMemory(hProcess, location, LibraryPath, strlen(LibraryPath) + 1, 0);
	if(!writeSuccess)
		ERROR("Failed to WriteProcessMemory")

	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, location, 0, 0);
	if(!hThread)
		ERROR("Failed to create remote thread for LoadLibraryW")

	WaitForSingleObject(hThread, INFINITE);

	DWORD ExitCode;
	GetExitCodeThread(hThread, &ExitCode);

	VirtualFreeEx(hProcess, location, MAX_PATH, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return (HMODULE)ExitCode;
}

BOOL Process::UnloadLibraryRemotely(DWORD ProcessID, std::wstring& moduleName)
{
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (!Snapshot)
		ERROR("Failed to create snapshot for remote process.")

	MODULEENTRY32W moduleEntry = { sizeof(moduleEntry) };
	
	bool found = false;
	BOOL bMoreModules = Module32FirstW(Snapshot, &moduleEntry);

	for (; bMoreModules; bMoreModules = Module32NextW(Snapshot, &moduleEntry))
	{
		std::wstring ModuleName(moduleEntry.szModule);

		found = (ModuleName == moduleName);
		if (found) break;
	}

	if (!found)
		ERROR("Failed to find module in remote process.")

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessID);

	if (!hProcess)
		ERROR("Failed to get handle for process.")

	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, moduleEntry.modBaseAddr, 0, 0);

	if (!hThread)
		ERROR("Failed to create remote thread for FreeLibrary")

	WaitForSingleObject(hThread, INFINITE);

	DWORD ExitCode;
	GetExitCodeThread(hThread, &ExitCode);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(Snapshot);

	return (BOOL)ExitCode;
}
