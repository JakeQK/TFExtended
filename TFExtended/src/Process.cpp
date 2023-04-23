#include "pch.h"
#include "Process.h"
#include "Log.h"

// This function takes the name of a process as input and returns its process ID (PID)
DWORD Process::GetProcessID(LPCTSTR ProcessName)
{
	// Create a snapshot of all running processes
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// Initialize a PROCESSENTRY32 structure with the size of the structure
	PROCESSENTRY32 pt;
	pt.dwSize = sizeof(PROCESSENTRY32);

	// Iterate through all running processes
	if (Process32First(hsnap, &pt)) 
	{
		do 
		{
			// Compare the executable file name of the current process with the given process name (case-insensitive)
			if (!lstrcmpi(pt.szExeFile, ProcessName)) 
			{
				// If the names match, close the snapshot handle and return the process ID of the current process
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} 
		while (Process32Next(hsnap, &pt));
	}

	// If no process with the given name is found, close the snapshot handle and return 0
	CloseHandle(hsnap);
	return 0;
}

// This function loads a library into a remote process by calling LoadLibraryA in a remote thread
HMODULE Process::LoadLibraryRemotely(DWORD ProcessID, const char* LibraryPath)
{
	// Open a handle to the remote process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessID);

	// Check if OpenProcess was successful
	if (!hProcess)
	{
		TFE_ERROR("Failed to get handle for process.");
		return NULL;
	}

	// Allocate memory in the remote process to store the library path
	LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(LibraryPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// Check if memory allocation was successful
	if (pDllPath == nullptr) 
	{
		TFE_ERROR("Failed to allocate memory in remote process");
		CloseHandle(hProcess);
		return NULL;
	}

	// Write the library path to the allocated memory in the remote process
	BOOL writeSuccess = WriteProcessMemory(hProcess, pDllPath, LibraryPath, strlen(LibraryPath) + 1, 0);

	// Check if writing to remote process's memory was successful
	if (!writeSuccess) 
	{
		TFE_ERROR("Failed to WriteProcessMemory");
		VirtualFreeEx(hProcess, pDllPath, strlen(LibraryPath) + 1, MEM_RELEASE);
		CloseHandle(hProcess);
		return NULL;
	}

	// Create a remote thread in the remote process that calls LoadLibraryA function with the path to the library
	HANDLE hThread = CreateRemoteThreadEx(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"), pDllPath, 0, nullptr, nullptr);

	// Check if remote thread creation was successful
	if (!hThread)
	{
		TFE_ERROR("Failed to create remote thread for LoadLibraryA");
		VirtualFreeEx(hProcess, pDllPath, strlen(LibraryPath) + 1, MEM_RELEASE);
		CloseHandle(hProcess);
		return NULL;
	}

	// Wait for the remote thread to complete its execution
	WaitForSingleObject(hThread, INFINITE);

	// Get the exit code of the remote thread, which is the handle to the loaded library
	DWORD ExitCode;
	GetExitCodeThread(hThread, &ExitCode);

	// Free the allocated memory in the remote process
	VirtualFreeEx(hProcess, pDllPath, strlen(LibraryPath) + 1, MEM_RELEASE);

	// Close the handle to the remote thread and the remote process
	CloseHandle(hThread);
	CloseHandle(hProcess);

	// Return the handle to the loaded library in the remote process
	return (HMODULE)ExitCode;
}

// This function fress a library loaded in a remote process by calling FreeLibrary in a remote thread
BOOL Process::FreeLibraryRemotely(DWORD ProcessID, HMODULE hModule)
{
	// Open a handle to the remote process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessID);

	// Check if OpenProcess was successful
	if (!hProcess) 
	{
		TFE_ERROR("Failed to get handle for process.");
		return false;
	}

	// Create a remote thread in the remote process that calls FreeLibrary function with the handle to the loaded library

	LPVOID freeLibraryAddress = GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");

	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)freeLibraryAddress, (LPVOID)hModule, 0, 0);



	// Check if remote thread creation was successful
	if (!hThread) 
	{
		TFE_ERROR("Failed to create remote thread for FreeLibrary");
		CloseHandle(hProcess);
		return false;
	}

	// Wait for the remote thread to complete its execution
	WaitForSingleObject(hThread, INFINITE);

	// Get the exit code of the remote thread, which should be the return value of FreeLibrary function
	DWORD ExitCode;
	GetExitCodeThread(hThread, &ExitCode);

	// Close the handle to the remote thread and the remote process
	CloseHandle(hThread);
	CloseHandle(hProcess);

	// Return whether FreeLibrary function was successful in the remote process
	return (BOOL)ExitCode;
}

// This function searches for module by name and Frees it remotely
BOOL Process::FreeLibraryRemotely(DWORD ProcessID, std::wstring& moduleName)
{
	// Create a snapshot of the remote process to get information about its modules
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);

	// Check if snapshot was successful
	if (!Snapshot)
	{
		TFE_ERROR("Failed to create snapshot for remote process.");
		return false;
	}

	// Initialize a MODULEENTRY32W structure with the size of the structure
	MODULEENTRY32W moduleEntry = { sizeof(moduleEntry) };
	
	// Iterate through all modules in the remote process until a module with the given name is found
	bool found = false;
	BOOL bMoreModules = Module32FirstW(Snapshot, &moduleEntry);

	for (; bMoreModules; bMoreModules = Module32NextW(Snapshot, &moduleEntry))
	{
		// Convert the module name to a wide string for comparison
		std::wstring ModuleName(moduleEntry.szModule);

		// Compare the module name of the current module with the given module name 
		found = (ModuleName == moduleName);
		if (found) break;
	}

	// If no module with the given name is found, log an error and return false;
	if (!found)
	{
		TFE_ERROR("Failed to find module in remote process.");
		CloseHandle(Snapshot);
		return false;
	}

	// Call FreeLibraryRemotely to free the module from the remote process and return the result
	BOOL ret = FreeLibraryRemotely(ProcessID, (HMODULE)moduleEntry.modBaseAddr);

	// Close the snapshot handle
	CloseHandle(Snapshot);
	return ret;
}
