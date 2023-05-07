#include "pch.h"
#include "utility.h"

namespace Utility
{
	DWORD GetTFProcessID()
	{
		PROCESSENTRY32 pt;
		HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		pt.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hsnap, &pt)) { // must call this first
			do {
				if (!lstrcmpi(pt.szExeFile, L"trials_fusion.exe")) {
					CloseHandle(hsnap);
					return pt.th32ProcessID;
				}
			} while (Process32Next(hsnap, &pt));
		}
		CloseHandle(hsnap); // close handle on failure
		return 0;
	}

	uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
	{
		uintptr_t modBaseAddr = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 modEntry;
			modEntry.dwSize = sizeof(modEntry);
			if (Module32First(hSnap, &modEntry))
			{
				do
				{
					if (!_wcsicmp(modEntry.szModule, modName))
					{
						modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
						break;
					}
				} while (Module32Next(hSnap, &modEntry));
			}
		}
		CloseHandle(hSnap);
		return modBaseAddr;
	}

	template<size_t N>
	int GetPointerAddress(HANDLE hProc, uintptr_t ptr, int offset_count, int offsets[N])
	{
		uintptr_t address = ptr;
		for (int i = 0; i < offset_count; i++)
		{
			ReadProcessMemory(hProc, (LPCVOID)address, &address, sizeof(address), NULL);
			address += offsets[i];
		}

		return address;
	}

	int GetPointerAddress(HANDLE hProc, uintptr_t ptr, int offset_count, ...)
	{
		va_list vl;
		va_start(vl, offset_count);

		uintptr_t address = ptr;
		for (int i = 0; i < offset_count; i++)
		{
			ReadProcessMemory(hProc, (LPCVOID)address, &address, sizeof(address), NULL);
			address += va_arg(vl, unsigned int);
		}

		va_end(vl);

		return address;
	}
};