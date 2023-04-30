#pragma once

#include "Menu.h"

#include "minhook/include/MinHook.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Log.h"

typedef void (*PresentCallback_t)();

__declspec(dllexport) void registerPresentCallback(PresentCallback_t callback);

__declspec(dllexport) void unregisterPresentCallback(PresentCallback_t callback);

namespace D3D11Hook
{
	static HWND	g_hWnd = nullptr;

	HRESULT __stdcall hSwapChainPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

	DXGI_SWAP_CHAIN_DESC CreateDummySwapchainDesc();

	void HookD3D11Present();

	void Initialize(std::unique_ptr<PluginManager>& pluginManager);
}