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
	//Disable left clicking if GUI is open, and focused or not allowing input passthrough
	if (GUI::DrawGUI && (GUI::ImGuiWindowFocused || !FrameworkInstance->InputPassthrough)) {
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
	MH_STATUS minHookStatus = MH_CreateHookApi(L"User32.dll", "SetCursorPos", &SetCursorPosHook, reinterpret_cast<LPVOID*>(&Original_SetCursorPos));
	if (minHookStatus != MH_OK) {
		std::string error = MH_StatusToString(minHookStatus);
		FrameworkInstance->LogMessage("[GUI] Failed to Create the SetCursorPos Hook, With the Error: " + error, TygerFramework::Error);
		return false;
	}
	//Hook GetKeyboardState
	minHookStatus = MH_CreateHookApi(L"User32.dll", "GetKeyboardState", &GetKeyboardStateHook, reinterpret_cast<LPVOID*>(&Original_GetKeyboardState));
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

	FrameworkInstance->LogMessage("[GUI] Hooked Mouse Input");
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
	FrameworkInstance->LogMessage("[GUI] Successfully Got Ty Window Handle");

	//Hook all the stuff needed to disable mouse input
	HookInput();

	//Setup ImGui Context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplWin32_InitForOpenGL(TyWindowHandle);
    ImGui_ImplOpenGL3_Init();

	//Don't set it to false here or it breaks and never shows the cursor
	if (GUI::DrawGUI)
		ShowCursor(true);

	Initialized = true;
	return true;
}

void GUI::Draw() {
	if (!GUI::DrawGUI)
		return;

	//Check if any imgui windows are focused for the mouse click hook
	GUI::ImGuiWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Set the window size once, just to update it to make sure its not too small
	//when using the saved size and cutting off options when there is more options added
	ImGui::SetNextWindowSize(ImVec2(285, 175), ImGuiCond_::ImGuiCond_Once);
	ImGui::Begin("TygerFramework");
	ImGui::Text("Menu Key: F1");
	ImGui::Checkbox("Show Console", &FrameworkInstance->ShowConsole);
	ImGui::Checkbox("Remember Menu Visibility", &FrameworkInstance->RememberVisibility);
	ImGui::Checkbox("Allow Input Passthrough", &FrameworkInstance->InputPassthrough);
	ImGui::SameLine();
	ImGui::Text("(?)");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Allows the game to register mouse clicks while the menu is open.");
	
	FrameworkInstance->PluginLoader.DrawUI();

	ImGui::End();
	//ImGui::ShowDemoWindow(&GUI::DrawGUI);

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

		//Only run when the GUI is shown (so it doesn't unfocus the imgui window when its hidden)
		if (GUI::DrawGUI)
		{
			//Pass WndProc to imgui (pass it in first due to potential "minimal evaluation" of the or in the if)
			//Stop the game from registering mouse movement for the camera when the GUI is open
			if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) || msg == WM_INPUT)
				return true;
		}
	}

	//Skip ImGui's win proc if not initialized or isn't a ImGui one
	return CallWindowProcA(Original_Wndproc, hWnd, msg, wParam, lParam);
}