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

static LPVOID*					g_pSwapChainVTable = nullptr;

static std::once_flag			g_isPresentInitialized;

typedef HRESULT(__stdcall* SwapChainPresent_t) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// Function pointer for the original Present function
SwapChainPresent_t oPresent = nullptr;

// Swap Chain Present Hook
// It is called whenever the Present funciton is called by the application
HRESULT __stdcall hSwapChainPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	std::call_once(g_isPresentInitialized, [&]() {
		pSwapChain->GetDevice(__uuidof(g_pd3dDevice), reinterpret_cast<LPVOID*>(&g_pd3dDevice));
		g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

		});

	// Call the original Present function using the stored function pointer oPresent
	return oPresent(pSwapChain, SyncInterval, Flags);
}

// Creates a dummy swap chain descriptor
DXGI_SWAP_CHAIN_DESC CreateDummySwapchainDesc()
{
	DXGI_SWAP_CHAIN_DESC sd{ 0 };
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Height = 800;
	sd.BufferDesc.Width = 600;
	sd.BufferDesc.RefreshRate = { 60, 1 };
	sd.OutputWindow = GetForegroundWindow();
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	return sd;
}

// Initializes the Direct3D 11 Swap Chain Present Hook
DWORD WINAPI InitD3D11Hook()
{
	D3D_FEATURE_LEVEL featureLevel;
	// Get dummy swap chain descriptor
	DXGI_SWAP_CHAIN_DESC sd = CreateDummySwapchainDesc();

	// Create a dummy device and swap chain
	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dContext);
	if (FAILED(hr)) {
		TEERROR("Failed to create device and swapchain")
	}

	// Get the VTable for the swap chain and the Present function
	g_pSwapChainVTable = *(LPVOID**)g_pSwapChain;
	SwapChainPresent_t pSwapChainPresent = (SwapChainPresent_t)(g_pSwapChainVTable[8]);

	// Hook the Present function using MinHook
	if (MH_Initialize() != MH_OK) { TEERROR("Failed to initialize MinHook") }
	if (MH_CreateHook(pSwapChainPresent, &hSwapChainPresent, reinterpret_cast<LPVOID*>(&oPresent)) != MH_OK) { TEERROR("Failed to create Present Hook") }
	if (MH_EnableHook(pSwapChainPresent) != MH_OK) { TEERROR("Failed to enable Present Hook") }

	// Return 1 if successful
	return 1;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitD3D11Hook, 0, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

