#include "pch.h"
#include "Process.h"

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
	WriteProcessMemory(hProcess, location, LibraryPath, strlen(LibraryPath) + 1, 0);

	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, location, 0, 0);

	WaitForSingleObject(hThread, INFINITE);

	DWORD ExitCode;
	GetExitCodeThread(hThread, &ExitCode);

	VirtualFreeEx(hProcess, location, MAX_PATH, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return (HMODULE)ExitCode;
}
