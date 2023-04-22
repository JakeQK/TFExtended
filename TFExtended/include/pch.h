// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H
#define WIN32_LEAN_AND_MEAN

#define TEERROR(msg) {}
#define TEINFO(msg) {}

// add headers that you want to pre-compile here
#include <windows.h>
#include <minwindef.h>
#include <string>
#include <TlHelp32.h>
#include <filesystem>
#include <iostream>
#include <format>
#include <algorithm>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace fs = std::filesystem;

#endif //PCH_H
