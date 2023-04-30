#pragma once
#include "imgui.h"

namespace InputHook
{
	void Initialize(HWND hWindow);
	void Shutdown();
};

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);