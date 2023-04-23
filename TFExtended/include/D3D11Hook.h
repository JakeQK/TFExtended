#pragma once

#include "TFExtendedMenu.h"

#include "minhook/include/MinHook.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Log.h"

namespace D3D11Hook
{
	static HWND	g_hWnd = nullptr;

	typedef void (*ImGuiCallback)();

	void registerImGuiCallback(ImGuiCallback callback);

	void unregisterImGuiCallback(ImGuiCallback callback);

	HRESULT __stdcall hSwapChainPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

	DXGI_SWAP_CHAIN_DESC CreateDummySwapchainDesc();

	void HookD3D11Present();

	void Initialize(std::unique_ptr<PluginManager>& pluginManager);
}