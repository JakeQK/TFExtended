#pragma once

namespace Utility
{
	// Get Process ID by Process Name
	DWORD GetProcessID(LPCTSTR ProcessName);

	// Load library into remote process from Path
	HMODULE LoadLibraryRemotely(DWORD ProcessID, const char* LibraryPath);

	// Free library from remote process by handle
	BOOL FreeLibraryRemotely(DWORD ProcessID, HMODULE handle);

	// Free library from remote process by module name
	BOOL FreeLibraryRemotely(DWORD ProcessID, std::wstring& Library);
};

