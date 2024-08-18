#include "pch.h"
#include "GUI.h"
#include "TygerFramework.h"
#include "WinUser.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

static WNDPROC Original_Wndproc;
LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool GUI::Init() {

	LPCSTR tyWindowName = "";

	//Switch based on which game is running to look for the correct window handle
	switch (FrameworkInstance->WhichTyGame()) {
	case 1:
		tyWindowName = "TY the Tasmanian Tiger";
		break;
	case 2:
		tyWindowName = "TY the Tasmanian Tiger 2: Bush Rescue";
		break;
	case 3:
		tyWindowName = "TY3";
		break;
	}
	TyWindowHandle = FindWindowA(NULL, tyWindowName);

	if (!TyWindowHandle) {
		FrameworkInstance->LogMessage("[GUI] Failed to Get Ty Window Handle", TygerFramework::Error);
		return false;
	}
	Original_Wndproc = (WNDPROC)SetWindowLongPtrW(TyWindowHandle, GWLP_WNDPROC, (LONG_PTR)WndProc);

	FrameworkInstance->LogMessage("[GUI] Successfully Got Ty Window Handle");

	//Setup ImGui Context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplWin32_InitForOpenGL(TyWindowHandle);
    ImGui_ImplOpenGL3_Init();

	//Temp
	ShowCursor(true);

	Initialized = true;
	return true;
}

void GUI::Draw() {
	if (!GUI::DrawUI)
		return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::Begin("TygerFramework");

	//ImGui::Text("Hello From TygerFramework");

	//ImGui::End();
	ImGui::ShowDemoWindow(&GUI::DrawUI);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (GUI::Initialized)
	{
		//Toggle ImGui
		if (msg == WM_KEYDOWN && wParam == VK_F1) {
			GUI::DrawUI = !GUI::DrawUI;
			ShowCursor(GUI::DrawUI);
		}

		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;
	}

	//Skip ImGui's win proc if not initialized or isn't a ImGui one
	return CallWindowProcA(Original_Wndproc, hWnd, msg, wParam, lParam);
}