#include "pch.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include <fstream>
#include <filesystem>
#include "imgui.h"
#include "APIHandler.h"
#include "Fonts/RobotoMedium.hpp"
#include "GUI.h"

namespace tygerFramework {
    void LogPluginMessage(std::string message, LogLevel logLevel) {
        FrameworkInstance->LogMessage(message, (TygerFramework::LogLevel)logLevel);
    }

    int WhichTyGame() {
        return FrameworkInstance->WhichTyGame();
    }
}

TygerFrameworkPluginFunctions pluginFunctions{
    tygerFramework::LogPluginMessage,
    tygerFramework::WhichTyGame,
    TygerFrameworkDrawPluginUi,
    TygerFrameworkPluginImguiHasFocus,
    TygerFrameworkPluginWndProc,
    TygerFrameworkGetTyWindowHandle,
    TygerFrameworkSetImGuiFont,
    PluginSetTygerFrameworkImGuiElements
};

TygerFrameworkPluginInitializeParam pluginInitParam{
    nullptr,
    "",
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

        pluginInitParam.pluginFileName = pluginName;
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

//Draw the loaded plugins section
void PluginLoader::DrawUI() {
    if (ImGui::CollapsingHeader("Plugins")) {

        if (mPlugins.empty()) {
            ImGui::Text("No Plugins Loaded");
            return;
        }
        ImGui::Text("Loaded Plugins:");
        ImGui::TreePush("Plugins");
        for (auto&& [name, _] : mPlugins) {
            ImGui::Text(name.c_str());
        }

        ImGui::TreePop();
    }
}

//Sets all the elements for drawing elements from the plugin in the TygerFramework window
//Overwrites the old value if the same plugin calls it again
void PluginSetTygerFrameworkImGuiElements(std::string pluginName, std::vector<TygerFrameworkImGuiParam> param)
{
    FrameworkInstance->PluginLoader.PluginImGuiElements.insert_or_assign(pluginName, param);
}

//Render any of the plugin imgui elements for the TygerFramework window
void PluginLoader::PluginDrawInTygerFrameworkWindow()
{
    for (auto&& [pluginName, elements] : PluginImGuiElements) {
        for (TygerFrameworkImGuiParam param : elements) {
            switch (param.ImGuiElement) {
            case CollapsingHeader:
                //Return it if the text is blank
                if (param.Text == "") {
                    FrameworkInstance->LogMessage("[" + pluginName + "]" + " Error, missing text for CollapsingHeader TygerFramework ImGui function! Returning Early", TygerFramework::Error);
                    return;
                }

                //Return out if its closed
                if (!ImGui::CollapsingHeader(param.Text.c_str()))
                    return;
                break;
            case Text:
                ImGui::Text(param.Text.c_str());
                break;
            case SameLine:
                ImGui::SameLine();
                break;
            case SetTooltip:
                //Skip it if the text is blank
                if (param.Text == "") {
                    FrameworkInstance->LogMessage("[" + pluginName + "]" + " Error, missing text for SetTooltip TygerFramework ImGui function! Skipping", TygerFramework::Error);
                    break;
                }

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(param.Text.c_str());
                break;
            case TreePush:
                //Skip it if the text is blank
                if (param.Text == "") {
                    FrameworkInstance->LogMessage("[" + pluginName + "]" + " Error, missing text for TreePush TygerFramework ImGui function! Skipping", TygerFramework::Error);
                    break;
                }

                ImGui::TreePush(param.Text.c_str());
                break;
            case TreePop:
                ImGui::TreePop();
                break;
            }
        }
    }
}

//ImGui context to send to plugins
HWND TygerFrameworkGetTyWindowHandle() {
    return FrameworkInstance->TyWindowHandle;
}

//Sets the plugin's ImGui's font to match the one the loader uses
void TygerFrameworkSetImGuiFont(void* imguiFont)
{
    ImFontAtlas* fonts = (ImFontAtlas*)imguiFont;
    fonts->Clear();

    ImFontConfig custom_icons{};
    custom_icons.FontDataOwnedByAtlas = false;

    fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, GUI::FontSize);
    fonts->Build();
}

bool TygerFrameworkPluginImguiHasFocus(ImGuiHasFocusFunc func)
{
    if (func == nullptr)
        return false;

    return APIHandler::Get()->AddPluginImGuiHasFocusFunc(func);
}

//Plugin draw function subscriber
bool TygerFrameworkDrawPluginUi(DrawUIFunc func)
{
    if (func == nullptr)
        return false;

    return APIHandler::Get()->AddDrawPluginUIFunc(func);
}

//Plugin WndProc function subscriber
bool TygerFrameworkPluginWndProc(WndProcFunc func) {

    if (func == nullptr)
        return false;

    return APIHandler::Get()->AddPluginWndProcFunc(func);
}
