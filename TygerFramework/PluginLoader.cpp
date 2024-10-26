#include "pch.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include <fstream>
#include <filesystem>
#include <format>
#include "imgui.h"
#include "APIHandler.h"
#include "Fonts/RobotoMedium.hpp"
#include "GUI.h"

namespace tygerFramework {
    void LogPluginMessage(std::string message, LogLevel logLevel) {
        FrameworkInstance->LogMessage(message, (TygerFramework::LogLevel)logLevel);
    }

    int CurrentTyGame() {
        return FrameworkInstance->CurrentTyGame();
    }
}

TygerFrameworkPluginFunctions pluginFunctions{
    tygerFramework::LogPluginMessage,
    tygerFramework::CurrentTyGame,
    TygerFrameworkDrawPluginUi,
    TygerFrameworkPluginImguiWantCaptureMouse,
    TygerFrameworkPluginWndProc,
    TygerFrameworkGetTyWindowHandle,
    TygerFrameworkDrawingGUI,
    TygerFrameworkSetImGuiFont,
    PluginSetTygerFrameworkImGuiElements,
    TygerFrameworkTickBeforeGame,
    TygerFrameworkOnTyInitialized,
    TygerFrameworkOnTyBeginShutdown,
    PluginSetTyBlockedInputProxy,
    GetPluginsTyBlockedInputState
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
    if (!fs::exists(pluginPath) && !fs::create_directories(pluginPath)) {
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
                mPluginErrors.emplace(path.stem().string(), "Failed to Load");
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
    pluginInitParam.TyHModule = FrameworkInstance->getFrameworkModule();

    for (auto plugins = mPlugins.begin(); plugins != mPlugins.end();) {
        std::string pluginName = plugins->first;
        HMODULE pluginModule = plugins->second;
        auto pluginRequiredVersionFunc = (TyFPluginRequiredVersion)GetProcAddress(pluginModule, "TygerFrameworkPluginRequiredVersion");

        //If the plugin doesn't implement the function it just skip it
        if (pluginRequiredVersionFunc == nullptr) {
            FrameworkInstance->LogMessage("[Plugin Loader] " + pluginName + " Doesn't Have a TygerFrameworkPluginRequiredVersion Function, Skipping");

            ++plugins;
            continue;
        }

        TygerFrameworkPluginVersion requiredVersion{};

        try {
            pluginRequiredVersionFunc(&requiredVersion);
        }
        catch (...) {
            FrameworkInstance->LogMessage("[Plugin Loader] " + pluginName + "Had An Exception Occur In TygerFrameworkPluginRequiredVersion, Skipping", TygerFramework::Error);
            mPluginErrors.emplace(pluginName, "An Exception Occured In TygerFrameworkPluginRequiredVersion");
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
        }

        //Check if the plugin requires a specific game
        if (requiredVersion.CompatibleGames.size() != 0 && std::find(std::begin(requiredVersion.CompatibleGames), std::end(requiredVersion.CompatibleGames), FrameworkInstance->CurrentTyGame()) == std::end(requiredVersion.CompatibleGames)) {
            FrameworkInstance->LogMessage(std::format("[Plugin Loader] {} Is Incompatible with Ty {}", pluginName, FrameworkInstance->CurrentTyGame()), TygerFramework::Error);
            mPluginErrors.emplace(pluginName, std::format("Incompatible with Ty {}", FrameworkInstance->CurrentTyGame()));
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
        }

        //Check which major and minor version is needed
        if (requiredVersion.Major > TygerFrameworkPluginVersion_Major || requiredVersion.Minor > TygerFrameworkPluginVersion_Minor) {
            FrameworkInstance->LogMessage(std::format("[Plugin Loader] {} Requires TygerFramework Version {}.{}.{} or Newer, But Version {}.{}.{} is Installed", 
                                                       pluginName, 
                                                       requiredVersion.Major, requiredVersion.Minor, requiredVersion.Patch, //Plugin Required Version
                                                       TygerFrameworkPluginVersion_Major, TygerFrameworkPluginVersion_Minor, TygerFrameworkPluginVersion_Patch), TygerFramework::Error); //Loader Version
            mPluginErrors.emplace(pluginName, std::format("Requires TygerFramework Version {}.{}.{} or Newer",
                                                           requiredVersion.Major, requiredVersion.Minor, requiredVersion.Patch));
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
        }

        //Warn about the patch version
        //Need to check the minor version so that if the plugin needs a version like 1.1.2 and the loader is on 1.2.0 this doesn't give a false warning
        if (requiredVersion.Patch > TygerFrameworkPluginVersion_Patch && requiredVersion.Minor == TygerFrameworkPluginVersion_Minor) {
            FrameworkInstance->LogMessage(std::format("[Plugin Loader] {} Desires Atleast Patch version x.{}.{}", 
                                                       pluginName, 
                                                       requiredVersion.Minor, requiredVersion.Patch), TygerFramework::Warning);
            mPluginWarnings.emplace(pluginName, std::format("Desires Atleast Patch version x.{}.{}", requiredVersion.Minor, requiredVersion.Patch));
        }

        ++plugins;
    }


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
                if (pluginInitParam.initErrorMessage != "")
                {
                    FrameworkInstance->LogMessage("[Plugin Loader] Failed to Initialize: " + pluginName + ", With the Error: " + pluginInitParam.initErrorMessage, TygerFramework::Error);
                    mPluginErrors.emplace(pluginName, "Failed to Initialize: " + pluginInitParam.initErrorMessage);
                }
                else
                {
                    FrameworkInstance->LogMessage("[Plugin Loader] Failed to Initialize: " + pluginName + ", With No Error Message Provided", TygerFramework::Error);
                    mPluginErrors.emplace(pluginName, "Failed to Initialize: No Error Message Provided");
                }
                FreeLibrary(pluginModule);
                plugins = mPlugins.erase(plugins);
                continue;
            }
        }
        catch (...) {
            FrameworkInstance->LogMessage("[Plugin Loader] " + pluginName + "Had An Exception Occur In TygerFrameworkPluginInitialize, Skipping", TygerFramework::Error);
            mPluginErrors.emplace(pluginName, "An Exception Occured In TygerFrameworkPluginInitialize");
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

        if (!mPlugins.empty()) {
            ImGui::Text("Loaded Plugins:");
            ImGui::TreePush("Plugins");
            for (auto&& [name, _] : mPlugins) {
                ImGui::Text(name.c_str());
            }

            ImGui::TreePop();
        }
        else {
            ImGui::Text("No Plugins Loaded");
        }

        if (!mPluginErrors.empty()) {
            ImGui::Spacing();
            ImGui::Text("Plugin Errors:");

            ImGui::TreePush("Plugin Errors");
            for (auto&& [name, error] : mPluginErrors) {
                ImGui::TextWrapped("%s - %s", name.c_str(), error.c_str());
            }
            ImGui::TreePop();
        }

        if (!mPluginWarnings.empty()) {
            ImGui::Spacing();
            ImGui::Text("Plugin Warnings:");

            ImGui::TreePush("Plugin Warnings");
            for (auto&& [name, warning] : mPluginWarnings) {
                ImGui::TextWrapped("%s - %s", name.c_str(), warning.c_str());
            }
            ImGui::TreePop();
        }
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
        bool headerCollapsed = false;
        for (TygerFrameworkImGuiParam param : elements) {
            //Allows for multiple collapsing headers
            if (headerCollapsed && param.ImGuiElement != CollapsingHeader)
                continue;
            switch (param.ImGuiElement) {
            case CollapsingHeader:
                //Return it if the text is blank
                if (param.Text == "") {
                    FrameworkInstance->LogMessage("[" + pluginName + "]" + " Error, missing text for CollapsingHeader TygerFramework ImGui function! Returning Early", TygerFramework::Error);
                    return;
                }

                //Check if its closed
                headerCollapsed = !ImGui::CollapsingHeader(param.Text.c_str());
                break;
            case Text:
                ImGui::Text(param.Text.c_str());
                break;
            case TextWrapped:
                ImGui::TextWrapped(param.Text.c_str());
                break;
            case SameLine:
                ImGui::SameLine();
                break;
            case NewLine:
                ImGui::NewLine();
                break;
            case Spacing:
                ImGui::Spacing();
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

bool PluginSetTyBlockedInputProxy(std::string pluginName, TyBlockedInputsFlags flags) {
    return FrameworkInstance->PluginLoader.SetTyBlockedInputs(pluginName, flags);
}

TyBlockedInputsFlags GetPluginsTyBlockedInputState(std::string pluginName)
{
    return FrameworkInstance->PluginLoader.GetPluginTyInputFlags()[pluginName];
}

//Blocker name is the one who wants to block input, for plugins it'll be the plugin name, but TygerFramework also uses this
bool PluginLoader::SetTyBlockedInputs(std::string blockerName, TyBlockedInputsFlags flags)
{
    mPluginTyInputFlags.insert_or_assign(blockerName, flags);
    CombineTyBlockedInputs();
    return true;
}

void PluginLoader::CombineTyBlockedInputs()
{
    //Reset the flags first
    TyInputCombinedFlags = None;
    //Combine all the flags
    for (auto&& [_, flag] : mPluginTyInputFlags) {
        TyInputCombinedFlags |= flag;
    }
}

//ImGui context to send to plugins
HWND TygerFrameworkGetTyWindowHandle() {
    return FrameworkInstance->TyWindowHandle;
}

bool TygerFrameworkDrawingGUI()
{
    return GUI::DrawGUI;
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

bool TygerFrameworkPluginImguiWantCaptureMouse(std::string pluginName, ImGuiWantCaptureMouseFunc func)
{
    if (func == nullptr)
        return false;

    return APIHandler::Get()->AddPluginImGuiWantCaptureMouseFunc({pluginName, func});
}

//Plugin draw function subscriber
bool TygerFrameworkDrawPluginUi(std::string pluginName, VoidFunc func)
{
    if (func == nullptr)
        return false;

    return APIHandler::Get()->AddDrawPluginUIFunc({pluginName, func});
}

//Plugin WndProc function subscriber
bool TygerFrameworkPluginWndProc(std::string pluginName, WndProcFunc func) {

    if (func == nullptr)
        return false;

    return APIHandler::Get()->AddPluginWndProcFunc({pluginName, func});
}

bool TygerFrameworkTickBeforeGame(std::string pluginName, TickBeforeGameFunc func)
{
    if (func == nullptr)
        return false;

    APIHandler::Get()->AddTickBeforeGameFunc({pluginName, func});
    return true;
}

bool TygerFrameworkOnTyInitialized(std::string pluginName, VoidFunc func)
{
    if (func == nullptr)
        return false;

    APIHandler::Get()->AddOnTyInitializedFunc({ pluginName, func });
    return true;
}

bool TygerFrameworkOnTyBeginShutdown(std::string pluginName, VoidFunc func)
{
    if (func == nullptr)
        return false;

    APIHandler::Get()->AddOnTyBeginShutdownFunc({ pluginName, func });
    return true;
}
