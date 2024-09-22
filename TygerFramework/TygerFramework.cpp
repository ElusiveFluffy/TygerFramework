#include "pch.h"
#include "TygerFramework.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "MinHook.h"
#include "OpenGLHook.h"
#include "GUI.h"
#include "ini.h"
namespace fs = std::filesystem;

std::unique_ptr<TygerFramework> FrameworkInstance;

typedef void(WINAPI* OutputDebugString_t) (LPCSTR lpOutputString);
OutputDebugString_t Original_OutputDebugString;

std::filesystem::path TygerFramework::GetPluginDir() {
    return fs::current_path() / "Plugins";
}

void TygerFramework::ToggleConsoleVisibility() {
    if (IsWindowVisible(GetConsoleWindow()))
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    else
        ShowWindow(GetConsoleWindow(), SW_SHOW);
}

TygerFramework::TygerFramework(HMODULE tygerFrameworkModule)
    : mTygerFrameworkModule{tygerFrameworkModule}
{
    mLogger.open("TygerFrameworkLog.txt");
    CreateConsole();
    LogMessage("[TygerFramework] Logger Started");
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
            LogMessage("[TygerFramework] Invalid steam_appid found, may be unable to accurately detect which Ty game is running, checking exe name", TygerFramework::Warning);
            AttemptToDetectGameFromExe();
        }
    }
    else
    {
        //If you're running it on steam it should always have this but just in case
        LogMessage("[TygerFramework] steam_appid.txt not found, may be unable to accurately detect which Ty game is running, checking exe name", TygerFramework::Warning);
        AttemptToDetectGameFromExe();
    }

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
        std::cout << "[Ty Log] " << lpOutputString;
    Original_OutputDebugString(lpOutputString);
}

bool TygerFramework::HookTyDebugOutput()
{
    MH_STATUS minHookStatus = MH_CreateHookApi(L"Kernel32.dll", "OutputDebugStringA", &TyOutputDebugString, reinterpret_cast<LPVOID*>(&Original_OutputDebugString));
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        LogMessage("[Ty Log Hook] Failed to Create the Ty Log Hook, With the Error: " + error, TygerFramework::Error);
        return false;
    }

    minHookStatus = MH_EnableHook(MH_ALL_HOOKS);
    if (minHookStatus != MH_OK) {
        std::string error = MH_StatusToString(minHookStatus);
        LogMessage("[Ty Log Hook] Failed to Enable the Ty Log Hook, With the Error: " + error, TygerFramework::Error);
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
        LogMessage("[TygerFramework] Unable to detect which Ty game is running from the exe", TygerFramework::Warning);
    }
}

void TygerFramework::LogMessage(std::string message, LogLevel logLevel) {
    std::string logLevelString;
    
    if (mLogger.is_open())
    {
        switch (logLevel)
        {
        case TygerFramework::Info:
            logLevelString = "[Info] ";
            break;
        case TygerFramework::Warning:
            logLevelString = "[Warning] ";
            break;
        case TygerFramework::Error:
            logLevelString = "[Error] ";
            break;
        default:
            break;
        }
        mLogger << logLevelString << message << std::endl;
        std::cout << logLevelString << message << std::endl;
    }
    else {
        std::ofstream outfile("LoggerErrors.txt");
        
        outfile << "Error Logger Isn't Running!" << std::endl;
        
        outfile.close();
    }
}
