#pragma once
#pragma comment(lib, "TFExtended.lib")

#include <Windows.h>

#define IMPORT __declspec(dllimport)

typedef void (*ImGuiCallback)();

IMPORT void registerImGuiCallback(ImGuiCallback callback);

IMPORT void unregisterImGuiCallback(ImGuiCallback callback);

