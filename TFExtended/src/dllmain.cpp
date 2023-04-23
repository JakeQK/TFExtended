#include "pch.h"

#include "TFExtendedMenu.h"
#include "PluginManager.h"
#include "LogManager.h"
#include "D3D11Hook.h"
#include "InputHook.h"
#include "Process.h"
#include "Log.h"

#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "minhook/include/MinHook.h"
#include "imgui.h"

#define TFEXTENDED_VERSION_MAJOR 1
#define TFEXTENDED_VERSION_MINOR 0

LogManager							g_logManager;
std::unique_ptr<PluginManager>		g_pluginManager;

// Initializes TFExtended
DWORD WINAPI InitTFExtended()
{
	// trials_fusion.exe launches UbisoftGameLauncher.exe and then actually launches the game
	// if you don't put this check, it will launch DLL's twice
	if (Utility::GetProcessID(L"UbisoftGameLauncher.exe") == 0)
		return 0;

	// Initialize our Log Manager
	g_logManager.Initialize();
	TFE_INFO("TFExtended v{}.{}", TFEXTENDED_VERSION_MAJOR, TFEXTENDED_VERSION_MINOR);

	// Construct our PluginManager utilizing unique pointer
	g_pluginManager = std::make_unique<PluginManager>();

	// Initialize Input Hook
	InputHook::Initialize(GetForegroundWindow());

	// Initialize our D3D11 Hooks
	D3D11Hook::Initialize(g_pluginManager);

	// Keeps application alive until termination key is pressed
	static bool TerminateTFExtended = false;
	while (!TerminateTFExtended)
	{
		if (GetAsyncKeyState(VK_END) & 0x1)
			TerminateTFExtended = true;

		Sleep(500);
	}

	InputHook::Shutdown();
	g_logManager.Shutdown();
	return 0;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitTFExtended, 0, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}