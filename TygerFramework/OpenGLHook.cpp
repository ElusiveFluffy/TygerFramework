#include "pch.h"
#include "OpenGLHook.h"
#include "MinHook.h"
#include "TygerFramework.h"
#include "GUI.h"

typedef BOOL (WINAPI* wglSwapBuffers_t) (HDC hDC);
wglSwapBuffers_t Original_wglSwapBuffers;
wglSwapBuffers_t Target_wglSwapBuffers;

BOOL WINAPI wglSwapBuffers_Func(HDC hDC) {

	//ImGui
	if (!GUI::Initialized)
		GUI::Init();
	else
		GUI::Draw();

	//If the checkbox value is different than the visibility, toggle the console
	if ((IsWindowVisible(GetConsoleWindow()) == TRUE) != FrameworkInstance->ShowConsole)
		FrameworkInstance->ToggleConsoleVisibility();

	//To make sure its not set to false twice, otherwise it'll never show again even if this is set to true
	if ((FrameworkInstance->PluginLoader.TyInputCombinedFlags & TyShowCursor) == TyShowCursor != FrameworkInstance->CursorShown) {
		//Flip the variable and set it
		FrameworkInstance->CursorShown = !FrameworkInstance->CursorShown;
		ShowCursor(FrameworkInstance->CursorShown);
	}

	//Run the original function
	return Original_wglSwapBuffers(hDC);
}

//Returns false if fail
bool OpenGLHook::Hook() {

	MH_STATUS minHookStatus = MH_CreateHookApi(L"OPENGL32.dll", "wglSwapBuffers", &wglSwapBuffers_Func, reinterpret_cast<LPVOID*>(&Original_wglSwapBuffers));
	if (minHookStatus != MH_OK) {
		std::string error = MH_StatusToString(minHookStatus);
		FrameworkInstance->LogMessage("[OpenGL Hook] Failed to Create the OpenGL Hook, With the Error: " + error, TygerFramework::Error);
		return false;
	}

	minHookStatus = MH_EnableHook(MH_ALL_HOOKS);
	if (minHookStatus != MH_OK) {
		std::string error = MH_StatusToString(minHookStatus);
		FrameworkInstance->LogMessage("[OpenGL Hook] Failed to Enable the OpenGL Hook, With the Error: " + error, TygerFramework::Error);
		return false;
	}

	return true;
}