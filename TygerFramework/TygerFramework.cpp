#include "pch.h"
#include "TygerFramework.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "MinHook.h"
#include "OpenGLHook.h"
namespace fs = std::filesystem;

std::unique_ptr<TygerFramework> FrameworkInstance;

std::filesystem::path TygerFramework::GetPluginDir() {
    return fs::current_path() / "Plugins";
}

TygerFramework::TygerFramework(HMODULE tygerFrameworkModule)
    : mTygerFrameworkModule{tygerFrameworkModule}
{
    mLogger.open("TygerFrameworkLog.txt");
    CreateConsole();
    LogMessage("[TygerFramework] Logger Started");

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
            LogMessage("[TygerFramework] Ty " + std::to_string(TyGame) + " Sucessfully Detected");
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

    //Hook OpenGL Swap Buffers Function
    if (!OpenGLHook::Hook()) {
        LogMessage("[OpenGL Hook] Failed to Hook the OpenGL Swap Buffers Function", Error);
    }
    else {
        LogMessage("[OpenGL Hook] Sucessfully Hooked OpenGL Swap Buffers Function");
    }
}

void TygerFramework::Shutdown() {
    FreeConsole();

    //Shutdown MinHook and remove all hooks
    MH_Uninitialize();
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
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
