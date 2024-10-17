// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include <iostream>
#include <fstream>
#include <filesystem>
//Make it use xinput 9.1.0
#define _WIN32_WINNT 0x0500
#include <Xinput.h>
namespace fs = std::filesystem;

//Xinput DLL Handle
HMODULE pXInput = 0;

bool Load_XInput9_1_0() {
    //Check if its already loaded
    if (pXInput) {
        return true;
    }

    wchar_t buffer[MAX_PATH]{ 0 };
    if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
        // Load the original XInput9_1_0.dll
        if ((pXInput = LoadLibraryW((std::wstring{ buffer } + L"\\XInput9_1_0.dll").c_str())) == NULL) {
            return false;
        }
        return true;
    }

    return false;
}

//XInputSetState wrapper for xinput9.1.0.dll
EXTERN_C DWORD xinput_set_state(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
    //This needs to be done because when we include xinput.h,
    //It is a redefinition, so we assign an export by not using the original name
    #pragma comment(linker, "/EXPORT:XInputSetState=xinput_set_state")

    //Make sure its loaded
    Load_XInput9_1_0();
    return ((decltype(XInputSetState)*)GetProcAddress(pXInput, "XInputSetState"))(dwUserIndex, pVibration);
}

//XInputGetState wrapper for xinput9.1.0.dll
EXTERN_C DWORD WINAPI xinput_get_state(DWORD dwUserIndex, XINPUT_STATE* pState) {
    //This needs to be done because when we include xinput.h,
    //It is a redefinition, so we assign an export by not using the original name
    #pragma comment(linker, "/EXPORT:XInputGetState=xinput_get_state")

    //Make sure its loaded
    Load_XInput9_1_0();
    return ((decltype(XInputGetState)*)GetProcAddress(pXInput, "XInputGetState"))(dwUserIndex, pState);
}

void startupThread(HMODULE tygerFrameworkModule) {
    FrameworkInstance->PluginLoader.Initialize();
    //Start checking if the game has initialized. 
    //Do it after all the plugins have initialized so then if initialization took long for the plugins and the game is already 
    //initialized then don't need extra logic to run the API function
    //Also makes it so another thread isn't needed
    if (FrameworkInstance->CurrentTyGame() != 0)
        FrameworkInstance->CheckIfGameFinishInit();
}

BOOL APIENTRY DllMain(HANDLE handle, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        FrameworkInstance = std::make_unique<TygerFramework>(GetModuleHandle(NULL));
        //Early intilization for the plugins before the game window shows, runs on the same startup thread as the game and the game will wait for this to complete
        FrameworkInstance->PluginLoader.EarlyInit();
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startupThread, handle, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        FrameworkInstance->Shutdown();
        break;
    }

    return TRUE;
}

