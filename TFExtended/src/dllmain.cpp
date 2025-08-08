#include "pch.h"

#include "menu.h"
#include "plugin_manager.h"
#include "log_manager.h"
#include "d3d11_hook.h"
#include "process.h"
#include "log.h"

#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "minhook/include/MinHook.h"
#include "imgui.h"

#define TFEXTENDED_VERSION_MAJOR 1
#define TFEXTENDED_VERSION_MINOR 0

LogManager							g_logManager;
std::shared_ptr<PluginManager>		g_pluginManager;

__declspec(dllexport) void registerPlugin(HMODULE hModule, std::string name, callbackFunction_t presentCallback)
{
	g_pluginManager->registerPlugin(hModule, name, presentCallback);
}

__declspec(dllexport) void unregisterPlugin(HMODULE hModule)
{
	g_pluginManager->unregisterPlugin(hModule);
}

// Initializes TFExtended
DWORD WINAPI InitTFExtended()
{
	// trials_fusion.exe launches UbisoftGameLauncher.exe and then actually launches the game
	// if you don't put this check, it will launch DLL's twice
	if (Utility::GetProcessID(L"UbisoftGameLauncher.exe") == 0)
	{
		return 0;
	}

	// Initialize our Log Manager
	g_logManager.Initialize();
	TFE_INFO("TFExtended v{}.{}", TFEXTENDED_VERSION_MAJOR, TFEXTENDED_VERSION_MINOR);

	// Construct our PluginManager utilizing unique pointer
	g_pluginManager = std::make_shared<PluginManager>();

	g_pluginManager->LoadAllPlugins();

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

	g_logManager.Shutdown();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

		D3D11Hook::DemoD3D11Hook();

		//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitTFExtended, 0, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}