#include "pch.h"

#include "PluginManager.h"
#include "Process.h"

#include "vendor/spdlog/sinks/basic_file_sink.h"
#include "vendor/minhook/include/MinHook.h"

static HWND                     g_hWnd = nullptr;
static HMODULE					g_hModule = nullptr;
static ID3D11Device*			g_pd3dDevice = nullptr;
static ID3D11DeviceContext*		g_pd3dContext = nullptr;
static IDXGISwapChain*			g_pSwapChain = nullptr;

DWORD_PTR*						pSwapChainVTable = nullptr;
DWORD_PTR*						pDeviceVTable = nullptr;
DWORD_PTR*						pDeviceContextVTable = nullptr;

static std::once_flag			g_isPresentInitialized;

typedef HRESULT(__stdcall* tD3D11Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
tD3D11Present oPresent = nullptr;

HRESULT __stdcall PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	std::call_once(g_isPresentInitialized, [&]() {
		pSwapChain->GetDevice(__uuidof(g_pd3dDevice), reinterpret_cast<LPVOID*>(&g_pd3dDevice));
		g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

		});

	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI InitD3D11Hook()
{
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	DXGI_SWAP_CHAIN_DESC sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.Windowed = ((GetWindowLongPtr(g_hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		sd.BufferDesc.Width = 1;
		sd.BufferDesc.Height = 1;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 1;
	}

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &obtainedLevel, &g_pd3dContext);
	if (FAILED(hr))
	{
		TEERROR("Failed to create device and swapchain")
	}

	pSwapChainVTable = (DWORD_PTR*)(g_pSwapChain);
	pSwapChainVTable = (DWORD_PTR*)(pSwapChainVTable[0]);

	if (MH_Initialize() != MH_OK) { TEERROR("Failed to initialize MinHook") }
	if(MH_CreateHook((DWORD_PTR*)pSwapChainVTable[8], PresentHook, reinterpret_cast<LPVOID*>(&oPresent)) != MH_OK) {TEERROR("Failed to create Present Hook") }

	return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);

		auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");

		Process process;
		if (process.GetProcessID(L"trials_fusion.exe"))
		{
			logger->info("Trials Fusion :)");
			logger->flush();
		}
		else
		{
			logger->info("No Trials Fusion :(");
			logger->flush();
		}
	}
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

