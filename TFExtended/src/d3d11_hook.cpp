#include "pch.h"
#include "d3d11_hook.h"
#include <d3d11.h>
#include "dxgi1_2.h"

#define D3D11_PRESENT1_VTABLE_INDEX 22

//#define _DEBUG_CONSOLE_

#ifdef _DEBUG_CONSOLE_
#define DEBUG_LOG(...) std::cout << __VA_ARGS__ << std::endl
#define ENABLE_CONSOLE CreateConsoleAndRedirectIO()
#else
#define DEBUG_LOG(...) void(0)
#define ENABLE_CONSOLE void(0)
#endif

std::unique_ptr<InputHook> g_inputHook;

namespace D3D11Hook
{
	static HWND	g_hWnd = nullptr;
	std::unique_ptr<Menu>		g_mainMenu = nullptr;
	std::shared_ptr<PluginManager>		g_pluginManager = nullptr;
	static bool							g_isPresentInitialized = false;

	static ID3D11Device* g_pd3dDevice = nullptr;
	static ID3D11DeviceContext* g_pd3dContext = nullptr;
	static IDXGISwapChain* g_pSwapChain = nullptr;
	static LPVOID* g_pSwapChainVTable = nullptr;

	typedef HRESULT(__stdcall* Present1_t)(IDXGISwapChain1* pSwapChain, UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);
	Present1_t oPresent1 = nullptr;

	HRESULT __stdcall hkPresent1(IDXGISwapChain1* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
	{
		// Present Hook Initialization
		if (!g_isPresentInitialized)
		{
			g_isPresentInitialized = true;

			// Get Device and Context pointers
			pSwapChain->GetDevice(__uuidof(g_pd3dDevice), reinterpret_cast<LPVOID*>(&g_pd3dDevice));
			g_pd3dDevice->GetImmediateContext(&g_pd3dContext);
				
			// Get Swap Chain Description
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			// Get Output Window HWND
			g_hWnd = sd.OutputWindow;

			// Setup ImGui context
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

			// Setup ImGui style
			ImGui::StyleColorsDark();

			// Setup Platform/Renderer backends
			ImGui_ImplWin32_Init(sd.OutputWindow);
			ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

			// Initialize Input Hook
			g_inputHook = std::make_unique<InputHook>();
			g_inputHook->Initialize(sd.OutputWindow);

			TFE_INFO("Present1 hook successfully initialized");
		}

		if (GetAsyncKeyState(VK_F4) & 0x1)
		{
			g_mainMenu->ToggleMenuOpen();
			TFE_INFO("Menu toggle clicked");
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Render TFExtended Main Menu
		g_mainMenu->Render();

		// Call plugins present callbacks
		if (g_pluginManager->plugins.size())
		{
			for (auto& pair : g_pluginManager->plugins)
			{
				if (pair.second->presentCallback != nullptr)
					pair.second->presentCallback();
			}
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Call the original Present function using the stored function pointer oPresent
		return oPresent1(pSwapChain, SyncInterval, Flags, pPresentParameters);
	}

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

	int HookD3D11()
	{
		D3D_FEATURE_LEVEL featureLevel;
		DXGI_SWAP_CHAIN_DESC sd = CreateDummySwapchainDesc();

		// Create a dummy device and swap chain
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dContext);
		if (FAILED(hr))
		{
			return -1;
		}


		g_pSwapChainVTable = *(LPVOID**)g_pSwapChain;

		// Query for the ID3D11SwapChain1 interface
		IDXGISwapChain1* pSwapChain1 = nullptr;
		hr = g_pSwapChain->QueryInterface(__uuidof(IDXGISwapChain1), (void**)&pSwapChain1);

		if (SUCCEEDED(hr) && pSwapChain1 != nullptr)
		{
			void** pVTable = *(void***)pSwapChain1;
			oPresent1 = (Present1_t)pVTable[D3D11_PRESENT1_VTABLE_INDEX];

			if (MH_Initialize() != MH_OK) 
			{ 
				TFE_INFO("Failed to initialize MinHook");
				// Failed to initialize MinHook
				return -1;
			}

			TFE_INFO("MinHook initialized");

			if (MH_CreateHook((Present1_t)pVTable[D3D11_PRESENT1_VTABLE_INDEX], &hkPresent1, reinterpret_cast<void**>(&oPresent1)) != MH_OK)
			{
				TFE_INFO("Failed to create Present1 Hook");

				// Failed to create Present1 Hook
				return -1;
			}

			TFE_INFO("Present1 hook created");


			if (MH_EnableHook((Present1_t)pVTable[D3D11_PRESENT1_VTABLE_INDEX]) != MH_OK)
			{
				TFE_INFO("Failed to enable Present1 Hook");

				// Failed to enable Present1 Hook
				return -1;
			}

			TFE_INFO("Present1 hook enabled");

			// Clean up dummy resources
			pSwapChain1->Release();
			return 1;
		}
		else
		{
			return -1;
			// Handle the case where D3D11.1 is not supported or QueryInterface failed
		}
	}

	// This function creates a new console and redirects standard I/O streams to it.
	void CreateConsoleAndRedirectIO() {
		// Allocate a console for the calling process.
		if (AllocConsole()) {
			// Redirect standard input, output, and error streams to the new console.
			// This allows functions like printf, std::cout, and std::cerr to work.
			// We use freopen_s for security over freopen.
			FILE* pConsoleStream;
			freopen_s(&pConsoleStream, "CONIN$", "r", stdin);
			freopen_s(&pConsoleStream, "CONOUT$", "w", stdout);
			freopen_s(&pConsoleStream, "CONOUT$", "w", stderr);

			// Optional: Set the console window title.
			SetConsoleTitle(L"DLL Debug Console");

			// Optional: Synchronize C++ streams with C standard streams.
			// This is important for mixed I/O operations.
			std::ios::sync_with_stdio();

			// Print a welcome message to the new console.
			std::cout << "Debug console attached successfully." << std::endl;
		}
	}

	// This function detaches the console and cleans up.
	void FreeConsoleAndCloseIO() {
		// Restore original I/O streams before freeing the console.
		// This is good practice but not strictly required.
		// CloseHandle(GetStdHandle(STD_INPUT_HANDLE));
		// CloseHandle(GetStdHandle(STD_OUTPUT_HANDLE));
		// CloseHandle(GetStdHandle(STD_ERROR_HANDLE));

		// Free the console.
		FreeConsole();
	}

	void Initialize(std::shared_ptr<PluginManager>& pluginManager)
	{
		TFE_INFO("D3D11 Hook Initialized");
		ENABLE_CONSOLE;
		g_pluginManager = pluginManager;
		g_mainMenu = std::make_unique<Menu>(pluginManager);
		HookD3D11();
	}

	static ID3D11Device* DEMO_g_pd3dDevice = nullptr;
	static ID3D11DeviceContext* DEMO_g_pd3dContext = nullptr;
	static IDXGISwapChain* DEMO_g_pSwapChain = nullptr;
	static LPVOID* DEMO_g_pSwapChainVTable = nullptr;
	static bool	DEMO_g_isPresentInitialized = false;
	static bool DEMO_Window = true;

	Present1_t DEMO_oPresent1 = nullptr;

	HRESULT __stdcall DEMO_hkPresent1(IDXGISwapChain1* pSwapChain, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
	{
		// Present Hook Initialization
		if (!DEMO_g_isPresentInitialized)
		{
			DEMO_g_isPresentInitialized = true;

			// Get Device and Context pointers
			pSwapChain->GetDevice(__uuidof(DEMO_g_pd3dDevice), reinterpret_cast<LPVOID*>(&DEMO_g_pd3dDevice));
			DEMO_g_pd3dDevice->GetImmediateContext(&DEMO_g_pd3dContext);

			// Get Swap Chain Description
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			// Get Output Window HWND
			g_hWnd = sd.OutputWindow;

			// Setup ImGui context
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

			// Setup ImGui style
			ImGui::StyleColorsDark();

			// Setup Platform/Renderer backends
			ImGui_ImplWin32_Init(sd.OutputWindow);
			ImGui_ImplDX11_Init(DEMO_g_pd3dDevice, DEMO_g_pd3dContext);

			// Initialize Input Hook
			g_inputHook = std::make_unique<InputHook>();
			g_inputHook->Initialize(sd.OutputWindow);

			TFE_INFO("Present1 hook successfully initialized");
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Text("Hello World!");

		ImGui::ShowDemoWindow(&DEMO_Window);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Call the original Present function using the stored function pointer oPresent
		return DEMO_oPresent1(pSwapChain, SyncInterval, Flags, pPresentParameters);
	}


	void DemoD3D11Hook()
	{
		D3D_FEATURE_LEVEL featureLevel;
		DXGI_SWAP_CHAIN_DESC sd = CreateDummySwapchainDesc();

		// Create a dummy device and swap chain
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &DEMO_g_pSwapChain, &DEMO_g_pd3dDevice, &featureLevel, &DEMO_g_pd3dContext);
		if (FAILED(hr))
		{
			TFE_ERROR("Failed to create device and swapchain");
		}

		DEMO_g_pSwapChainVTable = *(LPVOID**)DEMO_g_pSwapChain;

		// Query for the ID3D11SwapChain1 interface
		IDXGISwapChain1* pSwapChain1 = nullptr;
		hr = DEMO_g_pSwapChain->QueryInterface(__uuidof(IDXGISwapChain1), (void**)&pSwapChain1);

		if (FAILED(hr) || pSwapChain1 == nullptr)
		{
			TFE_INFO("Failed to get SwapChain1");
		}

		TFE_INFO("Successfully retrieved SwapChain1");

		void** pVTable = *(void***)pSwapChain1;
		DEMO_oPresent1 = (Present1_t)pVTable[D3D11_PRESENT1_VTABLE_INDEX];

		if (MH_Initialize() != MH_OK)
		{
			TFE_INFO("Failed to initialize MinHook");
		}

		TFE_INFO("MinHook initialized");

		if (MH_CreateHook((Present1_t)pVTable[D3D11_PRESENT1_VTABLE_INDEX], &DEMO_hkPresent1, reinterpret_cast<void**>(&DEMO_oPresent1)) != MH_OK)
		{
			TFE_INFO("Failed to create Present1 Hook");
		}

		TFE_INFO("Present1 hook created");


		if (MH_EnableHook((Present1_t)pVTable[D3D11_PRESENT1_VTABLE_INDEX]) != MH_OK)
		{
			TFE_INFO("Failed to enable Present1 Hook");
		}

		TFE_INFO("Present1 hook enabled");

		// Clean up dummy resources
		pSwapChain1->Release();
	}
}