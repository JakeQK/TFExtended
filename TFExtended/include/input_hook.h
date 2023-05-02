#pragma once
#include "imgui.h"

class InputHook
{
public:
	void Initialize(HWND hWindow);
	void Shutdown();
};