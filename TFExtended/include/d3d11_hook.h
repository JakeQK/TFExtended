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
	void Initialize(std::shared_ptr<PluginManager>& pluginManager);

	int HookD3D11();

	void DemoD3D11Hook();
}