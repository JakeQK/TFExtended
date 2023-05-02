#pragma once
#pragma comment(lib, "TFExtended.lib")

#include <Windows.h>

#define IMPORT __declspec(dllimport)

typedef void (*PresentCallback_t)();

IMPORT void registerPresentCallback(PresentCallback_t callback);

IMPORT void unregisterPresentCallback(PresentCallback_t callback);