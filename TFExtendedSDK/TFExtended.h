#pragma once
#pragma comment(lib, "TFExtended.lib")

#include <Windows.h>
#include <string>

#define IMPORT __declspec(dllimport)

typedef void (*callbackFunction_t)();

IMPORT void registerPlugin(HMODULE hModule, std::string name, callbackFunction_t presentCallback);

IMPORT void unregisterPlugin(HMODULE hModule);