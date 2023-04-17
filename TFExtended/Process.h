#pragma once
class Process
{
public:
	// Constructor
	Process();

	// Destructor
	~Process();

	DWORD GetProcessID(LPCTSTR ProcessName);

	HMODULE LoadLibraryRemotely(DWORD ProcessID, const char* LibraryPath);
};

