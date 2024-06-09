#include "pch.h"
#include "TygerFramework.h"
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

std::unique_ptr<TygerFramework> FrameworkInstance;

std::filesystem::path TygerFramework::GetPluginDir() {
    return fs::current_path() / "Plugins";
}

TygerFramework::TygerFramework(HMODULE tygerFrameworkModule)
    : mTygerFrameworkModule{tygerFrameworkModule}
{
    mLogger.open("TygerFrameworkLog.txt");
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
    }
    else {
        std::ofstream outfile("LoggerErrors.txt");
        
        outfile << "Error Logger Isn't Running!" << std::endl;
        
        outfile.close();
    }
}
