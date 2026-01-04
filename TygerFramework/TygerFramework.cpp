#include "pch.h"
#include "TygerFramework.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <format>
#include "MinHook.h"
#include "OpenGLHook.h"
#include "GUI.h"
#include "TyMemoryValues.h"
#include "APIHandler.h"
#include "Logger.h"
#include "ini.h"
namespace fs = std::filesystem;
using namespace Logger;

std::unique_ptr<TygerFramework> FrameworkInstance;

typedef void(WINAPI* OutputDebugString_t) (LPCSTR lpOutputString);
OutputDebugString_t Original_OutputDebugString;

typedef int32_t(WINAPI* TyShutdown_t) ();
TyShutdown_t Original_TyShutdown;

int32_t WINAPI TyBeginShutdown() {
    LogMessage("[Ty Shutdown Hook] Ty Began Shutting Down");
    //Notify all plugins
    APIHandler::Get()->OnTyBeginShutdown();

    //Run the game's shutdown function
    return Original_TyShutdown();
}

std::filesystem::path TygerFramework::GetPluginDir() {
#ifdef _DEBUG
    return fs::current_path() / "Debug Plugins";
#endif // DEBUG
#ifdef NDEBUG
    return fs::current_path() / "Plugins";
#endif // RELEASE
}

std::filesystem::path TygerFramework::GetDependencyDir() {
    return GetPluginDir() / "Dependencies";
}

void TygerFramework::ToggleConsoleVisibility() {
    if (IsWindowVisible(GetConsoleWindow()))
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    else
        ShowWindow(GetConsoleWindow(), SW_SHOW);
}

TygerFramework::TygerFramework(HMODULE TyHModule)
    : mTyHModule{ TyHModule }
{
    Logger::StartLogger();
    CreateConsole();
    LogMessage("[TygerFramework] Logger Started");
    LogMessage(std::format("[TygerFramework] TygerFramework v{}.{}.{}", TygerFrameworkPluginVersion_Major, 
                                                                        TygerFrameworkPluginVersion_Minor, 
                                                                        TygerFrameworkPluginVersion_Patch));
    LoadSettings();

    if (fs::exists("steam_appid.txt"))
    {
        std::ifstream steamAppIDFile("steam_appid.txt");
        std::string AppID;
        std::getline(steamAppIDFile, AppID);
        steamAppIDFile.close();

        switch (std::stoi(AppID)) {
        case Ty1AppID:
            TyGame = 1;
            break;
        case Ty2AppID:
            TyGame = 2;
            break;
        case Ty3AppID:
            TyGame = 3;
            break;
        }
        if (TyGame != 0) {
            LogMessage("[TygerFramework] Ty " + std::to_string(TyGame) + " Detected");
        }
        else {
            LogMessage("[TygerFramework] Invalid steam_appid found, may be unable to accurately detect which Ty game is running, checking exe name", Warning);
            AttemptToDetectGameFromExe();
        }
    }
    else
    {
        //If you're running it on steam it should always have this but just in case
        LogMessage("[TygerFramework] steam_appid.txt not found, may be unable to accurately detect which Ty game is running, checking exe name", Warning);
        AttemptToDetectGameFromExe();
    }
    
    if (mTyHModule) {
        TyMemoryValues::TyBaseAddress = (DWORD)mTyHModule;
        LogMessage("[TygerFramework] Got Ty Base Address");

        TyMemoryValues::SetVersionText(TyGame);
    }
    else
        LogMessage("[TygerFramework] Couldn't get Ty base address", Error);


    //Setup MinHook
    MH_STATUS minhookStatus = MH_Initialize();
    if (minhookStatus != MH_OK) {
        std::string error = MH_StatusToString(minhookStatus);
        LogMessage("[TygerFramework] Failed to Initialize Minhook, With the Error: " + error, Error);
    }

    //Hook the output of the Ty Log
    if (!HookTyDebugOutput())
        LogMessage("[Ty Log Hook] Failed to Hook the Ty Log", Error);
    else
        LogMessage("[Ty Log Hook] Sucessfully Hooked the Ty Log");

    //Hook OpenGL Swap Buffers Function
    if (!OpenGLHook::Hook()) {
        LogMessage("[OpenGL Hook] Failed to Hook the OpenGL Swap Buffers Function", Error);
    }
    else {
        LogMessage("[OpenGL Hook] Sucessfully Hooked OpenGL Swap Buffers Function");
    }
}

//Simple way to get the Ty log output, rather than constantly checking if the txt file has changed
void WINAPI TyOutputDebugString(LPCSTR lpOutputString) {
    //Only log to the console if the option is enabled
    if (FrameworkInstance->TyLogInConsole)
        std::cout << GetTimeStamp() << "[Ty Log] " << lpOutputString;
    Original_OutputDebugString(lpOutputString);
}

bool TygerFramework::HookTyDebugOutput()
{
    MH_STATUS minHookStatus = MH_CreateHookApi(L"Kernel32.dll", "OutputDebugStringA", &TyOutputDebugString, reinterpret_cast<LPVOID*>(&Original_OutputDebugString));
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        LogMessage("[Ty Log Hook] Failed to Create the Ty Log Hook, With the Error: " + error, Error);
        return false;
    }

    minHookStatus = MH_EnableHook(MH_ALL_HOOKS);
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        LogMessage("[Ty Log Hook] Failed to Enable the Ty Log Hook, With the Error: " + error, Error);
        return false;
    }

    return true;
}

void TygerFramework::Shutdown() {
    SaveSettings();

    FreeConsole();

    //Shutdown MinHook and remove all hooks
    MH_Uninitialize();
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
}

void TygerFramework::SaveSettings() {
    ini::File settings;

    //Create TygerFramework section
    settings.add_section("TygerFramework");
    //Save the data, [Section name], (Value name, value)
    settings["TygerFramework"].set<bool>("ShowConsole", ShowConsole);
    settings["TygerFramework"].set<bool>("TyLogInConsole", TyLogInConsole);

    //Create GUI section
    settings.add_section("GUI");
    settings["GUI"].set<bool>("RememberVisibility", RememberVisibility);
    settings["GUI"].set<bool>("Visible", GUI::DrawGUI);
    settings["GUI"].set<bool>("InputPassthrough", InputPassthrough);
    settings["GUI"].set<bool>("KeyboardAlwaysPassthrough", KeyboardAlwaysPassthrough);

    //Save the settings
    settings.write("TygerFramework.ini");

    LogMessage("[TygerFramework] Saved Settings to ini");
}

void TygerFramework::LoadSettings() {
    if (!fs::exists("TygerFramework.ini")) {
        //Console is hidden by default, hide it here instead of CreateConsole(), 
        //to not have a slight flash when quickly reshowing it after loading settings
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        return;
    }

    ini::File settings = ini::open("TygerFramework.ini");

    //Tygerframework section
    if (settings.has_section("TygerFramework")) {
        ini::Section tygerFrameworkSection = settings["TygerFramework"];

        if (tygerFrameworkSection.has_key("ShowConsole")) {
            ShowConsole = tygerFrameworkSection.get<bool>("ShowConsole");
            //Hide the console here instead of flashng it by reshowing it. It needs to be created before
            //loading the settings to be able to log the message below to the console
            if (!ShowConsole)
                ShowWindow(GetConsoleWindow(), SW_HIDE);
        }
        if (tygerFrameworkSection.has_key("TyLogInConsole"))
            TyLogInConsole = tygerFrameworkSection.get<bool>("TyLogInConsole");
    }
    //GUI section
    if (settings.has_section("GUI")) {
        ini::Section GuiSection = settings["GUI"];

        if (GuiSection.has_key("RememberVisibility"))
            RememberVisibility = GuiSection.get<bool>("RememberVisibility");
        //Only set this if remembering visiblility
        if (RememberVisibility && GuiSection.has_key("Visible"))
            GUI::DrawGUI = GuiSection.get<bool>("Visible");
        if (GuiSection.has_key("InputPassthrough"))
            InputPassthrough = GuiSection.get<bool>("InputPassthrough");
        if (GuiSection.has_key("KeyboardAlwaysPassthrough"))
            KeyboardAlwaysPassthrough = GuiSection.get<bool>("KeyboardAlwaysPassthrough");
    }

    LogMessage("[TygerFramework] Loaded Settings from ini");

    settings.clear();
}

void TygerFramework::CreateConsole() {
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    //Reset the stream in case there is errors blocking it for some reason
    std::cout.clear();
}

void TygerFramework::AttemptToDetectGameFromExe() {
    WCHAR fullPath[MAX_PATH]{ 0 };
    GetModuleFileName(NULL, fullPath, MAX_PATH);
    fs::path exePath(fullPath);
    std::string exeName = exePath.stem().string();

    //Start from Ty 3 otherwise if starting with just "Ty" it'll find that in Ty 2 and 3
    if (exeName.find("TY3") != std::string::npos)
        TyGame = 3;
    else if (exeName.find("TY2") != std::string::npos)
        TyGame = 2;
    else if (exeName.find("TY") != std::string::npos)
        TyGame = 1;

    if (TyGame != 0) {
        LogMessage("[TygerFramework] Ty " + std::to_string(TyGame) + " Sucessfully Detected");
    }
    else {
        LogMessage("[TygerFramework] Unable to detect which Ty game is running from the exe", Warning);
    }
}

void TygerFramework::CheckIfGameFinishInit() {
    if (!TyMemoryValues::TyBaseAddress)
        return;
    //Just waiting for the game to startup, values below 5 are all uses before fully initialized
    while (TyMemoryValues::HasGameInitialized()) {
        Sleep(100);
    }
    TyHasInitialized = true;
    APIHandler::Get()->OnTyInitialized();
    HookTyShutdown();
}

bool TygerFramework::HookTyShutdown()
{
    //Hook Ty Shutdown Function
    MH_STATUS minHookStatus = MH_CreateHook(TyMemoryValues::GetTyShutdownFunc(), &TyBeginShutdown, reinterpret_cast<LPVOID*>(&Original_TyShutdown));
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        LogMessage("[Ty Shutdown Hook] Failed to Create the Ty Shutdown Function Hook, With the Error: " + error, Error);
        return false;
    }

    //Enable both hooks
    minHookStatus = MH_EnableHook(MH_ALL_HOOKS);
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        LogMessage("[Ty Shutdown Hook] Failed to Hook Ty Shutdown Function, With the Error: " + error, Error);
        return false;
    }

    LogMessage("[Ty Shutdown Hook] Sucessfully Hooked the Ty Shutdown Function");
    return true;
}

bool TygerFramework::SetTyInputFlag(TyInputsFlags flag, bool enableFlag)
{
    if (enableFlag)
        return PluginLoader.SetTyInputState("TygerFramework", (PluginLoader.GetPluginTyInputFlags()["TygerFramework"] | flag));
    else
        return PluginLoader.SetTyInputState("TygerFramework", (PluginLoader.GetPluginTyInputFlags()["TygerFramework"] & ~flag));
}