#include "pch.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include <fstream>
#include <filesystem>

void PluginLoader::EarlyInit() try {
    namespace fs = std::filesystem;

    std::ofstream pluginLog("PluginLoader.txt");

    pluginLog << "Plugin Loader Early Initialization Started" << std::endl;

    fs::path pluginPath = TygerFramework::GetPluginDir();
    //Create it if it doesn't exist
    if (!fs::create_directories(pluginPath) && !fs::exists(pluginPath)) {
        pluginLog << "Failed to Create Plugin Folder!" << std::endl;
        return;
    }
    else{
        pluginLog << "Loading Plugins From: " << pluginPath << std::endl;
    }

    for (auto&& entry : fs::directory_iterator{ pluginPath }) {
        auto&& path = entry.path();

        if (path.has_extension() && path.extension() == ".dll") {
            auto module = LoadLibrary(path.c_str());

            pluginLog << "Loading: " << path.filename().string() << std::endl;

            if (module == nullptr) {
                pluginLog << "Failed to load plugin: " << path.filename().string() << std::endl;
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