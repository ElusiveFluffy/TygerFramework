#include "pch.h"
#include "GUI.h"
#include "TygerFramework.h"
#include "WinUser.h"
#include "MinHook.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

static WNDPROC Original_Wndproc;
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//SetCursorPos Hook
typedef BOOL (WINAPI* SetCursorPos_t) (int X, int Y);
SetCursorPos_t Original_SetCursorPos;
//GetKeyboardState Hook
typedef BOOL (WINAPI* GetKeyboardState_t) (PBYTE lpKeyState);
GetKeyboardState_t Original_GetKeyboardState;

BOOL WINAPI SetCursorPosHook(int X, int Y) {
	//Unlock the cursor from the center of the screen (for when the game locks it)
	if (GUI::DrawGUI)
		return 1;

	return Original_SetCursorPos(X, Y);
}

BOOL WINAPI GetKeyboardStateHook(PBYTE lpKeyState) {
	//Disable left clicking if GUI is open
	if (GUI::DrawGUI) {
		//Get the current key state for everything else
		BOOL returnVal = Original_GetKeyboardState(lpKeyState);
		if (returnVal != 0) {
			//Disable the mouse buttons
			lpKeyState[VK_LBUTTON] = 0;
			lpKeyState[VK_MBUTTON] = 0;
			lpKeyState[VK_RBUTTON] = 0;
		}
		return returnVal;
	}

	return Original_GetKeyboardState(lpKeyState);
}

bool HookInput() {
	//Hook SetCursorPos
	MH_STATUS minHookStatus = MH_CreateHookApi(L"User32", "SetCursorPos", &SetCursorPosHook, reinterpret_cast<LPVOID*>(&Original_SetCursorPos));
	if (minHookStatus != MH_OK) {
		std::string error = MH_StatusToString(minHookStatus);
		FrameworkInstance->LogMessage("[GUI] Failed to Create the SetCursorPos Hook, With the Error: " + error, TygerFramework::Error);
		return false;
	}
	//Hook GetKeyboardState
	minHookStatus = MH_CreateHookApi(L"User32", "GetKeyboardState", &GetKeyboardStateHook, reinterpret_cast<LPVOID*>(&Original_GetKeyboardState));
	if (minHookStatus != MH_OK) {
		std::string error = MH_StatusToString(minHookStatus);
		FrameworkInstance->LogMessage("[GUI] Failed to Create the GetKeyboardState Hook, With the Error: " + error, TygerFramework::Error);
		return false;
	}

	//Enable both hooks
	minHookStatus = MH_EnableHook(MH_ALL_HOOKS);
	if (minHookStatus != MH_OK) {
		std::string error = MH_StatusToString(minHookStatus);
		FrameworkInstance->LogMessage("[GUI] Failed to Enable the Mouse Hooks, With the Error: " + error, TygerFramework::Error);
		return false;
	}

	return true;
}

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

	//Hook all the stuff needed to disable mouse input
	HookInput();

	FrameworkInstance->LogMessage("[GUI] Successfully Got Ty Window Handle");

	//Setup ImGui Context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplWin32_InitForOpenGL(TyWindowHandle);
    ImGui_ImplOpenGL3_Init();

	//Temp
	ShowCursor(GUI::DrawGUI);

	Initialized = true;
	return true;
}

void GUI::Draw() {
	if (!GUI::DrawGUI)
		return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::Begin("TygerFramework");

	//ImGui::Text("Hello From TygerFramework");

	//ImGui::End();
	ImGui::ShowDemoWindow(&GUI::DrawGUI);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (GUI::Initialized)
	{
		//Toggle ImGui
		if (msg == WM_KEYDOWN && wParam == VK_F1) {
			GUI::DrawGUI = !GUI::DrawGUI;
			ShowCursor(GUI::DrawGUI);
		}

		//Pass WndProc to imgui first
		LRESULT imGuiLResult = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

		//Stop the game from registering mouse movement for the camera when the GUI is open
		if (GUI::DrawGUI && msg == WM_INPUT || imGuiLResult)
			return true;
	}

	//Skip ImGui's win proc if not initialized or isn't a ImGui one
	return CallWindowProcA(Original_Wndproc, hWnd, msg, wParam, lParam);
}