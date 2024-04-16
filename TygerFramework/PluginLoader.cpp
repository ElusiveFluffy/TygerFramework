#include "pch.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include <fstream>
#include <filesystem>

void PluginLoader::EarlyInit() try {
    namespace fs = std::filesystem;

    std::ofstream pluginLog("PluginLoader.txt");

    FrameworkInstance->LogMessage("[Plugin Loader] Early Initialization Started");

    fs::path pluginPath = TygerFramework::GetPluginDir();
    //Create it if it doesn't exist
    if (!fs::create_directories(pluginPath) && !fs::exists(pluginPath)) {
        FrameworkInstance->LogMessage("[Plugin Loader] Failed to Create Plugin Folder!", TygerFramework::Error);
        return;
    }
    FrameworkInstance->LogMessage("[Plugin Loader] Loading Plugins From : " + pluginPath.string());

    for (auto&& entry : fs::directory_iterator{ pluginPath }) {
        auto&& path = entry.path();

        if (path.has_extension() && path.extension() == ".dll") {
            auto module = LoadLibrary(path.c_str());

            FrameworkInstance->LogMessage("[Plugin Loader] Loading: " + path.filename().string());

            if (module == nullptr) {
                FrameworkInstance->LogMessage("[Plugin Loader] Failed to Load Plugin: " + path.filename().string(), TygerFramework::Error);
                continue;
            }

            mPlugins.emplace(path.stem().string(), module);
        }
    }

    pluginLog.close();
}
catch (const std::exception& e) {
    
}
catch (...) {
    
}