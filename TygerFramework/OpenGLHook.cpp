#include "pch.h"
#include "OpenGLHook.h"
#include "MinHook.h"
#include "TygerFramework.h"
#include "GUI.h"

typedef BOOL (__stdcall* wglSwapBuffers_t) (HDC hDC);
wglSwapBuffers_t Original_wglSwapBuffers;
wglSwapBuffers_t Target_wglSwapBuffers;

BOOL __stdcall wglSwapBuffers_Func(HDC hDC) {
	//FrameworkInstance->LogMessage("Tick");

	if (!GUI::Initialized)
		GUI::Init();
	else
		GUI::Draw();

	//Run the original function
	return Original_wglSwapBuffers(hDC);
}

//Returns false if fail
bool OpenGLHook::Hook() {
	//FARPROC wglSwapBuffers_address = GetProcAddress(GetModuleHandleA("OPENGL32.dll"), "wglSwapBuffers");

	MH_STATUS minHookStatus = MH_CreateHookApi(L"OPENGL32", "wglSwapBuffers", &wglSwapBuffers_Func, reinterpret_cast<LPVOID*>(&Original_wglSwapBuffers));
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
	//MH_CreateHook(wglSwapBuffers_address, &wglSwapBuffers_Func, reinterpret_cast<LPVOID*>(&Original_wglSwapBuffers));
}