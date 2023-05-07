#pragma once

namespace Utility
{
	DWORD GetTFProcessID();
	uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
	template<size_t N>
	int GetPointerAddress(HANDLE hProc, uintptr_t ptr, int offset_count, int offsets[N]);
	int GetPointerAddress(HANDLE hProc, uintptr_t ptr, int offset_count, ...);
};