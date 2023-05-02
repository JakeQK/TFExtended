#pragma once

#include "menu.h"
#include "log.h"
#include "input_hook.h"

#include "minhook/include/MinHook.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

namespace D3D11Hook
{
	static HWND	g_hWnd = nullptr;

	HRESULT __stdcall hSwapChainPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

	DXGI_SWAP_CHAIN_DESC CreateDummySwapchainDesc();

	void HookD3D11Present();

	void Initialize(std::unique_ptr<PluginManager>& pluginManager);
}