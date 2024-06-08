#include "pch.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include <fstream>
#include <filesystem>

namespace tygerFramework {
    void LogPluginMessage(std::string message, LogLevel logType) {
        FrameworkInstance->LogMessage("[Plugin] " + message, (TygerFramework::LogLevel)logType);
    }

    int WhichTyGame() {
        return FrameworkInstance->WhichTyGame();
    }
}

TygerFrameworkPluginFunctions pluginFunctions{
    tygerFramework::LogPluginMessage,
    tygerFramework::WhichTyGame
};

TygerFrameworkPluginInitializeParam pluginInitParam{
    nullptr,
    &pluginFunctions
};

void PluginLoader::EarlyInit() try {
    namespace fs = std::filesystem;

    FrameworkInstance->LogMessage("[Plugin Loader] Early Initialization Started");

    fs::path pluginPath = TygerFramework::GetPluginDir();
    //Create it if it doesn't exist
    if (!fs::create_directories(pluginPath) && !fs::exists(pluginPath)) {
        FrameworkInstance->LogMessage("[Plugin Loader] Failed to Create Plugin Folder!", TygerFramework::Error);
        return;
    }
    FrameworkInstance->LogMessage("[Plugin Loader] Loading Plugins From: " + pluginPath.string());

    for (auto&& entry : fs::directory_iterator{ pluginPath }) {
        auto&& path = entry.path();

        if (path.has_extension() && path.extension() == ".dll") {
            auto module = LoadLibrary(path.c_str());

            FrameworkInstance->LogMessage("[Plugin Loader] Loading: " + path.stem().string());

            if (module == nullptr) {
                FrameworkInstance->LogMessage("[Plugin Loader] Failed to Load Plugin: " + path.stem().string(), TygerFramework::Error);
                continue;
            }

            mPlugins.emplace(path.stem().string(), module);
        }
    }
}
catch (const std::exception& e) {
    std::string message = "[Plugin Loader] Error Occured During Plugin Early Initilization: ";
    message += e.what();
    FrameworkInstance->LogMessage(message, TygerFramework::Error);
}
catch (...) {
    FrameworkInstance->LogMessage("[Plugin Loader] Unhandled Exception Occured During Plugin Early Initilization", TygerFramework::Error);
}

//Call TygerFrameworkPluginInitialize on any dlls that export it
void PluginLoader::Initialize() {

    //Set Tygerframework module
    pluginInitParam.tygerFrameworkModule = FrameworkInstance->getFrameworkModule();

    for (auto plugins = mPlugins.begin(); plugins != mPlugins.end();) {
        std::string pluginName = plugins->first;
        HMODULE pluginModule = plugins->second;
        auto pluginInitializer = (TyFPluginInitializer)GetProcAddress(pluginModule, "TygerFrameworkPluginInitialize");

        //Skip the plugin if it doesn't have the function
        if (pluginInitializer == nullptr) {
            ++plugins;
            continue;
        }

        FrameworkInstance->LogMessage("[Plugin Loader] Initializing: " + pluginName);
        try {
            if (!pluginInitializer(&pluginInitParam)) {
                FrameworkInstance->LogMessage("[Plugin Loader] Failed to Initialize: " + pluginName, TygerFramework::Error);
                FreeLibrary(pluginModule);
                plugins = mPlugins.erase(plugins);
                continue;
            }
        }
        catch (...) {
            FrameworkInstance->LogMessage("[Plugin Loader] " + pluginName + "Had An Exception Occur In TygerFrameworkPluginInitialize, Skipping", TygerFramework::Error);
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
        }

        ++plugins;
    }
}