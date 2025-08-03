// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#define EXPORT __declspec(dllexport)

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#define TARGET_LIBRARY "TFExtended.dll"

FARPROC pMiniDumpReadDumpStream = nullptr;
FARPROC pMiniDumpWriteDump = nullptr;

extern "C" {
	EXPORT void MiniDumpReadDumpStream() { __asm { jmp pMiniDumpReadDumpStream } }
	EXPORT void MiniDumpWriteDump() { __asm { jmp pMiniDumpWriteDump } }
}

void Initialize()
{
	char path[MAX_PATH];
	GetSystemDirectoryA(path, MAX_PATH);
	strcpy_s(path, "\\dbgcore.dll");
	HMODULE hModule = LoadLibraryA(path);

	pMiniDumpReadDumpStream = GetProcAddress(hModule, "MiniDumpReadDumpStream");
	pMiniDumpWriteDump = GetProcAddress(hModule, "MiniDumpWriteDump");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		Initialize();

		LoadLibraryA(TARGET_LIBRARY);
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

