#include "pch.h"
#include "TygerFramework.h"
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

std::unique_ptr<TygerFramework> FrameworkInstance;

std::filesystem::path TygerFramework::GetPluginDir() {
    return fs::current_path() / "Plugins";
}

int TygerFramework::WhichTyGame() {
    return TyGame;
}

TygerFramework::TygerFramework(HMODULE tygerFrameworkModule)
    : mTygerFrameworkModule{tygerFrameworkModule}
{
    mLogger.open("TygerFrameworkLog.txt");
    LogMessage("Logger Started");

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
            LogMessage("Ty " + std::to_string(TyGame) + " Sucessfully Detected");
        }
        else {
            LogMessage("Invalid steam_appid found, may be unable to accurately detect which Ty game is running, checking exe name", TygerFramework::Warning);
            AttemptToDetectGameFromExe();
        }
    }
    else
    {
        //If you're running it on steam it should always have this but just in case
        LogMessage("steam_appid.txt not found, may be unable to accurately detect which Ty game is running, checking exe name", TygerFramework::Warning);
        AttemptToDetectGameFromExe();
    }

    //Early intilization for the plugins before the game window shows, runs on the same startup thread as the game and the game will wait for this to complete
    //mPluginLoader.EarlyInit();
}

void TygerFramework::AttemptToDetectGameFromExe() {
    WCHAR fullPath[MAX_PATH]{ 0 };
    GetModuleFileName(NULL, fullPath, MAX_PATH);
    fs::path exePath(fullPath);
    std::string exeName = exePath.stem().string();

    if (exeName == "TY")
        TyGame = 1;
    else if (exeName == "TY2")
        TyGame = 2;
    else if (exeName == "TY3")
        TyGame = 3;

    if (TyGame != 0) {
        LogMessage("Ty " + std::to_string(TyGame) + " Sucessfully Detected");
    }
    else {
        LogMessage("Unable to detect which Ty game is running from the exe", TygerFramework::Warning);
    }
}

void TygerFramework::LogMessage(std::string message, LogLevel logType) {
    std::string logTypeString;
    
    if (mLogger.is_open())
    {
        switch (logType)
        {
        case TygerFramework::Info:
            logTypeString = "[Info] ";
            break;
        case TygerFramework::Warning:
            logTypeString = "[Warning] ";
            break;
        case TygerFramework::Error:
            logTypeString = "[Error] ";
            break;
        default:
            break;
        }
        mLogger << logTypeString << message << std::endl;
    }
    else {
        std::ofstream outfile("LoggerErrors.txt");
        
        outfile << "Error Logger Isn't Running!" << std::endl;
        
        outfile.close();
    }
}
