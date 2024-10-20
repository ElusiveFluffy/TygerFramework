#include "pch.h"
#include "GUI.h"
#include "TygerFramework.h"
#include "MinHook.h"
#include "APIHandler.h"
#include "TygerFrameworkAPI.h"
#include <format>

#include "Fonts/RobotoMedium.hpp"
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
		BOOL returnVal = 1;
		if (!FrameworkInstance->KeyboardAlwaysPassthrough)
		{
			//Clears the entire keyboard and mouse state buffer (the buffer is always 256 bytes long)
			//For a full clear the original function doesn't need to be run
			//If not running the original function need to clear the memory here otherwise there is 
			//just a bunch of garbage data at the pointer that the game will try to read as inputs
			memset(lpKeyState, 0, 256);
		}
		else
		{
			//Get the current key state for everything else
			returnVal = Original_GetKeyboardState(lpKeyState);
			if (returnVal != 0) {
				//Disable the mouse buttons
				lpKeyState[VK_LBUTTON] = 0;
				lpKeyState[VK_MBUTTON] = 0;
				lpKeyState[VK_RBUTTON] = 0;
			}

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
	switch (FrameworkInstance->CurrentTyGame()) {
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
	FrameworkInstance->TyWindowHandle = FindWindowA(NULL, tyWindowName);

	if (!FrameworkInstance->TyWindowHandle) {
		FrameworkInstance->LogMessage("[GUI] Failed to Get Ty Window Handle", TygerFramework::Error);
		return false;
	}
	Original_Wndproc = (WNDPROC)SetWindowLongPtrW(FrameworkInstance->TyWindowHandle, GWLP_WNDPROC, (LONG_PTR)WndProc);
	FrameworkInstance->LogMessage("[GUI] Successfully Got Ty Window Handle");

	//Hook all the stuff needed to disable mouse input
	HookInput();

	//Setup ImGui Context
	ImGui::CreateContext();

	SetImGuiStyle();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui_ImplWin32_InitForOpenGL(FrameworkInstance->TyWindowHandle);
	ImGui_ImplOpenGL3_Init();

	//Don't set it to false here or it breaks and never shows the cursor
	if (GUI::DrawGUI)
		ShowCursor(true);

	Initialized = true;
	return true;
}

void GUI::SetImGuiStyle() {
	ImFontAtlas* fonts = ImGui::GetIO().Fonts;
	fonts->Clear();

	ImFontConfig custom_icons{};
	custom_icons.FontDataOwnedByAtlas = false;

	fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, GUI::FontSize);
	fonts->Build();
}

float LastFrameFPS = 0;

void GUI::Draw() {

	//Need to be called before NewFrame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	//New fps average calculated to be used for the tick
	ImGui::NewFrame();

	//Weird ImGui draw loop so the ImGui's framerate function can be used, which has a rolling average fps which is more accurate
	//Check if the framerate is higher than 10 (mainly incase you tabbed out of the game), if it is less then 10 then use the last frame fps
	if (ImGui::GetIO().Framerate > 10) {
		APIHandler::Get()->TickBeforeGame(1.0f / ImGui::GetIO().Framerate);
		LastFrameFPS = ImGui::GetIO().Framerate;
	}
	else
		APIHandler::Get()->TickBeforeGame(1.0f / LastFrameFPS);

	//Draw ImGui windows
	if (GUI::DrawGUI) {
		//Draw all the plugin windows
		APIHandler::Get()->DrawPluginUI();

		//Check if any imgui windows are focused for the mouse click hook
		GUI::ImGuiWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || APIHandler::Get()->PluginImGuiWantCaptureMouse();
		//Set the window size once, just to update it to make sure its not too small
		//when using the saved size and cutting off options when there is more options added
		ImGui::SetNextWindowSize(ImVec2(285, 240), ImGuiCond_::ImGuiCond_Once);
		//The ### allows a internal name to be specified after it, so when the version changes it doesn't reset the saved position of the window
		ImGui::Begin(std::format("TygerFramework v{}.{}.{}###TygerFramework", TygerFrameworkPluginVersion_Major, TygerFrameworkPluginVersion_Minor, TygerFrameworkPluginVersion_Patch).c_str());
		ImGui::Text("Menu Key: F1");
		ImGui::Checkbox("Show Console", &FrameworkInstance->ShowConsole);
		ImGui::Checkbox("Show Ty Log in the Console", &FrameworkInstance->TyLogInConsole);
		ImGui::SameLine();
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Output's the game's log to the console too.");
		ImGui::Spacing();
		ImGui::Checkbox("Remember Menu Visibility", &FrameworkInstance->RememberVisibility);
		ImGui::Checkbox("Allow Input Passthrough", &FrameworkInstance->InputPassthrough);
		ImGui::SameLine();
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Allows the game to register mouse clicks and keyboard input while the menu is open but not focused.");

		ImGui::Checkbox("Keyboard Always Passthrough", &FrameworkInstance->KeyboardAlwaysPassthrough);
		ImGui::SameLine();
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Allows the game to register keyboard input even while the menu is focused.");
		FrameworkInstance->PluginLoader.DrawUI();
		FrameworkInstance->PluginLoader.PluginDrawInTygerFrameworkWindow();


		ImGui::End();

		ImGui::ShowDemoWindow();
	}
	//Needs to be called after NewFrame
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
			//Run the WndProcs both here so they will get run, so 1 doesn't not get run because of "minimal evaluation" of the or in the if
			LRESULT WndProcResult = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
			//Plugin WndProc for if the plugin wants to block any WndProc from the game
			bool pluginWndProcResult = APIHandler::Get()->PluginWndProc(hWnd, msg, wParam, lParam);
			//Stop the game from registering mouse movement for the camera when the GUI is open
			if (WndProcResult || pluginWndProcResult || msg == WM_INPUT)
				return true;
		}
		//Allow WndProc events to still be sent for plugins even when the UI is hidden
		//Plugins need to handle checking if the UI is hidden
		else if (APIHandler::Get()->PluginWndProc(hWnd, msg, wParam, lParam))
			return true;
	}

	//Skip ImGui's win proc if not initialized or isn't a ImGui one
	return CallWindowProcA(Original_Wndproc, hWnd, msg, wParam, lParam);
}