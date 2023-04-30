#include "pch.h"
#include "input_hook.h"

WNDPROC oWndProc;
HWND hWnd = nullptr;

namespace InputHook
{
	void Initialize(HWND hWindow)
	{
		if (hWnd == nullptr) {
			hWnd = hWindow;
			oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
		}
	}

	void Shutdown()
	{
		if (hWnd != nullptr) {
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
			hWnd = nullptr;
		}
	}
};

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