#include "pch.h"
#include "D3D11Hook.h"

std::vector<PresentCallback_t> g_presentCallbacks;

// Adds ImGuiCallback functions to ImGuiCallbacks vector to be called within Present Hook
void registerPresentCallback(PresentCallback_t callback)
{
	TFE_DEBUG("registerImGuiCallback called!");
	// Ensure callback isn't already part of the vector, if not, add it
	if (std::find(g_presentCallbacks.begin(), g_presentCallbacks.end(), callback) == g_presentCallbacks.end())
	{
		g_presentCallbacks.push_back(callback);
		TFE_INFO("{} callback registered.", fmt::ptr(callback))
	}
}

void unregisterPresentCallback(PresentCallback_t callback)
{
	TFE_DEBUG("unregisterImGuiCallback called!");
	// Finds callback in ImGuiCallbacks vector and removes it if found
	auto callbackIterator = std::find(g_presentCallbacks.begin(), g_presentCallbacks.end(), callback);
	if (callbackIterator != g_presentCallbacks.end())
	{
		g_presentCallbacks.erase(callbackIterator);
		TFE_INFO("{} callback unregistered.", fmt::ptr(callback));
	}
}


namespace D3D11Hook
{
	std::unique_ptr<Menu>		g_mainMenu = nullptr;
	static ID3D11Device*				g_pd3dDevice = nullptr;
	static ID3D11DeviceContext*			g_pd3dContext = nullptr;
	static IDXGISwapChain*				g_pSwapChain = nullptr;
	static LPVOID*						g_pSwapChainVTable = nullptr;

	static std::once_flag				g_isPresentInitialized;

	typedef HRESULT(__stdcall* SwapChainPresent_t) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	SwapChainPresent_t					oPresent = nullptr;

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

			// Setup ImGui context
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

			// Setup ImGui style
			ImGui::StyleColorsDark();

			// Setup Platform/Renderer backends
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
		if (!g_presentCallbacks.empty())
		{
			for (PresentCallback_t callback : g_presentCallbacks)
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
		sd.OutputWindow = GetForegroundWindow();
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

	void Initialize(std::unique_ptr<PluginManager>& pluginManager)
	{
		g_mainMenu = std::make_unique<Menu>(pluginManager);
		HookD3D11Present();
	}
}