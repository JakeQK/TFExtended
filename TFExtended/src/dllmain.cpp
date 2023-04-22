#include "pch.h"

#include "LogManager.h"
#include "Log.h"
#include "PluginManager.h"
#include "TFExtendedMenu.h"
#include "Process.h"

#include "minhook/include/MinHook.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

static HWND							g_hWnd = nullptr;
static HMODULE						g_hModule = nullptr;
static ID3D11Device*				g_pd3dDevice = nullptr;
static ID3D11DeviceContext*			g_pd3dContext = nullptr;
static IDXGISwapChain*				g_pSwapChain = nullptr;
static LPVOID*						g_pSwapChainVTable = nullptr;
static WNDPROC						oWndProc;

LogManager							g_logManager;
std::unique_ptr<PluginManager>		g_pluginManager;
std::unique_ptr<TFExtendedMenu>		g_mainMenu;

static std::once_flag				g_isPresentInitialized;

typedef HRESULT(__stdcall* SwapChainPresent_t) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// Function pointer for the original Present function
SwapChainPresent_t					oPresent = nullptr;

typedef void (*ImGuiCallback)();
std::vector<ImGuiCallback>			ImGuiCallbacks;

// WndProc forward declaration
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Adds ImGuiCallback functions to ImGuiCallbacks vector to be called within Present Hook
__declspec(dllexport) void registerImGuiCallback(ImGuiCallback callback)
{
	TFE_DEBUG("registerImGuiCallback called!");
	// Ensure callback isn't already part of the vector, if not, add it
	if (std::find(ImGuiCallbacks.begin(), ImGuiCallbacks.end(), callback) == ImGuiCallbacks.end()) 
	{
		ImGuiCallbacks.push_back(callback);
		TFE_INFO("{} callback registered.", fmt::ptr(callback))
	}
}

__declspec(dllexport) void unregisterImGuiCallback(ImGuiCallback callback)
{
	TFE_DEBUG("unregisterImGuiCallback called!");
	// Finds callback in ImGuiCallbacks vector and removes it if found
	auto callbackIterator = std::find(ImGuiCallbacks.begin(), ImGuiCallbacks.end(), callback);
	if (callbackIterator != ImGuiCallbacks.end())
	{
		ImGuiCallbacks.erase(callbackIterator);
		TFE_INFO("{} callback unregistered.", fmt::ptr(callback));
	}
}

// Swap Chain Present Hook
// It is called whenever the Present funciton is called by the application
HRESULT __stdcall hSwapChainPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	// Present Hook Initialization
	std::call_once(g_isPresentInitialized, [&]() {
		// Get Device and Context pointers
		pSwapChain->GetDevice(__uuidof(g_pd3dDevice), reinterpret_cast<LPVOID*>(&g_pd3dDevice));
		g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

		// Get Swap Chain Description
		DXGI_SWAP_CHAIN_DESC sd;
		pSwapChain->GetDesc(&sd);
		// Get Output Window HWND
		g_hWnd = sd.OutputWindow;

		// Set window procedure to our own (WndProc) and store original in oWndProc
		oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

		// Create unique pointers for objects
		g_pluginManager = std::make_unique<PluginManager>();
		g_mainMenu = std::make_unique<TFExtendedMenu>(g_pluginManager);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(g_hWnd);
		ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

		TFE_INFO("Swap chain present hook initialized.");

		});

	if (GetAsyncKeyState(VK_F1) & 0x1)
		g_mainMenu->ToggleMenuOpen();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Render TFExtended Main Menu
	g_mainMenu->Render();

	// Render ImGuiCallback functions
	if (!ImGuiCallbacks.empty())
	{
		for (ImGuiCallback callback : ImGuiCallbacks)
		{
			callback();
		}
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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
	sd.OutputWindow = g_hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	return sd;
}

// Creates the D3D11 Present Hook
void HookD3D11Present()
{
	D3D_FEATURE_LEVEL featureLevel;
	// Get dummy swap chain descriptor
	DXGI_SWAP_CHAIN_DESC sd = CreateDummySwapchainDesc();

	// Create a dummy device and swap chain
	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dContext);
	if (FAILED(hr))
		TFE_ERROR("Failed to create device and swap chain")

	// Get the VTable for the swap chain and the Present function
	g_pSwapChainVTable = *(LPVOID**)g_pSwapChain;
	SwapChainPresent_t pSwapChainPresent = (SwapChainPresent_t)(g_pSwapChainVTable[8]);

	// Hook the Present function using MinHook
	if (MH_Initialize() != MH_OK) { TFE_ERROR("Failed to initialize MinHook") }
	if (MH_CreateHook(pSwapChainPresent, &hSwapChainPresent, reinterpret_cast<LPVOID*>(&oPresent)) != MH_OK) { TFE_ERROR("Failed to create Present Hook") }
	if (MH_EnableHook(pSwapChainPresent) != MH_OK) { TFE_ERROR("Failed to enable Present Hook") }
}

// Initializes TFExtended
DWORD WINAPI InitTFExtended()
{
	g_logManager.Initialize();
	TFE_INFO("TFExtended v{}.{}", 1, 0)

	g_hWnd = GetForegroundWindow();

	HookD3D11Present();

	// Keeps application alive until termination key is pressed
	static bool TerminateTFExtended = false;
	while (!TerminateTFExtended)
	{
		if (GetAsyncKeyState(VK_END) & 0x1)
			TerminateTFExtended = true;

		Sleep(500);
	}

	// Restores original window procedure
	SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
	return 0;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitTFExtended, 0, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		ImGui::GetIO().MouseDown[0] = true; return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	case WM_LBUTTONUP:
		ImGui::GetIO().MouseDown[0] = false; return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	case WM_RBUTTONDOWN:
		ImGui::GetIO().MouseDown[1] = true; return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	case WM_RBUTTONUP:
		ImGui::GetIO().MouseDown[1] = false; return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	case WM_MBUTTONDOWN:
		ImGui::GetIO().MouseDown[2] = true; return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	case WM_MBUTTONUP:
		ImGui::GetIO().MouseDown[2] = false; return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	case WM_MOUSEWHEEL:
		ImGui::GetIO().MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f; return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	case WM_MOUSEMOVE:
		ImGui::GetIO().MousePos.x = (signed short)(lParam); ImGui::GetIO().MousePos.y = (signed short)(lParam >> 16); return DefWindowProc(hwnd, uMsg, wParam, lParam);

		break;
	}

	return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}