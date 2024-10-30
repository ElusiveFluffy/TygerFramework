#include "pch.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include <fstream>
#include <filesystem>
#include <format>
#include "imgui.h"
#include "APIHandler.h"
#include "Logger.h"
#include "Fonts/RobotoMedium.hpp"
#include "GUI.h"
namespace fs = std::filesystem;

namespace tygerFramework {
    int CurrentTyGame() {
        return FrameworkInstance->CurrentTyGame();
    }
}

TygerFrameworkPluginFunctions pluginFunctions{
    Logger::LogMessage,
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
    PluginSetTyInputStateProxy,
    GetPluginsTyInputState,
    TygerFramework::GetPluginDir
};

TygerFrameworkPluginInitializeParam pluginInitParam{
    nullptr,
    "",
    &pluginFunctions
};

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString(DWORD errorCode)
{
    if (errorCode == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    //Remove the new line
    message.pop_back();
    message.pop_back();

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

void PluginLoader::DependencyInit() try {

    fs::path dependencyPath = TygerFramework::GetDependencyDir();
    //Create it if it doesn't exist
    if (!fs::exists(dependencyPath) && !fs::create_directories(dependencyPath)) {
        Logger::LogMessage("[Plugin Loader] Failed to Create Dependency Folder!", Error);
        return;
    }
    Logger::LogMessage("[Plugin Loader] Loading Dependencies From: " + dependencyPath.string());

    for (auto&& entry : fs::directory_iterator{ dependencyPath }) {
        auto&& path = entry.path();

        if (path.has_extension() && path.extension() == ".dll") {
            auto module = LoadLibrary(path.c_str());

            Logger::LogMessage("[Plugin Loader] Loading: " + path.stem().string());

            if (module == nullptr) {
                DWORD errorCode = GetLastError();
                std::string errorMessage = GetLastErrorAsString(errorCode);
                Logger::LogMessage("[Plugin Loader] Failed to Load Dependency: " + path.stem().string() + ", With the Error: " + errorMessage, Error);
                mDependencyErrors.emplace(path.stem().string(), "Failed to Load With the Error: " + errorMessage);
                continue;
            }

            mDependencies.emplace(path.stem().string(), module);
        }
    }
}
catch (const std::exception& e) {
    std::string message = "[Plugin Loader] Error Occured During Dependency Initilization: ";
    message += e.what();
    Logger::LogMessage(message, Error);
}
catch (...) {
    Logger::LogMessage("[Plugin Loader] Unhandled Exception Occured During Dependency Initilization", Error);
}

void PluginLoader::PluginEarlyInit() try {

    fs::path pluginPath = TygerFramework::GetPluginDir();
    //Create it if it doesn't exist
    if (!fs::exists(pluginPath) && !fs::create_directories(pluginPath)) {
        Logger::LogMessage("[Plugin Loader] Failed to Create Plugin Folder!", Error);
        return;
    }
    Logger::LogMessage("[Plugin Loader] Loading Plugins From: " + pluginPath.string());

    for (auto&& entry : fs::directory_iterator{ pluginPath }) {
        auto&& path = entry.path();

        if (path.has_extension() && path.extension() == ".dll") {
            auto module = LoadLibrary(path.c_str());

            Logger::LogMessage("[Plugin Loader] Loading: " + path.stem().string());

            if (module == nullptr) {
                DWORD errorCode = GetLastError();
                std::string errorMessage = GetLastErrorAsString(errorCode);
                Logger::LogMessage("[Plugin Loader] Failed to Load Plugin: " + path.stem().string() + ", With the Error: " + errorMessage + "(" + std::to_string(errorCode) + ")", Error);
                mPluginErrors.emplace(path.stem().string(), "Failed to Load: " + errorMessage + "(" + std::to_string(errorCode) + ")");
                continue;
            }

            mPlugins.emplace(path.stem().string(), module);
        }
    }
}
catch (const std::exception& e) {
    std::string message = "[Plugin Loader] Error Occured During Plugin Early Initilization: ";
    message += e.what();
    Logger::LogMessage(message, Error);
}
catch (...) {
    Logger::LogMessage("[Plugin Loader] Unhandled Exception Occured During Plugin Early Initilization", Error);
}

void PluginLoader::EarlyInit() {
    Logger::LogMessage("[Plugin Loader] Early Initialization Started");

    //Initialize Dependencies first
    DependencyInit();
    PluginEarlyInit();
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
            Logger::LogMessage("[Plugin Loader] " + pluginName + " Doesn't Have a TygerFrameworkPluginRequiredVersion Function, Skipping");

            ++plugins;
            continue;
        }

        TygerFrameworkPluginVersion requiredVersion{};

        try {
            pluginRequiredVersionFunc(&requiredVersion);
        }
        catch (...) {
            Logger::LogMessage("[Plugin Loader] " + pluginName + "Had An Exception Occur In TygerFrameworkPluginRequiredVersion, Skipping", Error);
            mPluginErrors.emplace(pluginName, "An Exception Occured In TygerFrameworkPluginRequiredVersion");
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
        }

        //Check if the plugin requires a specific game
        if (requiredVersion.CompatibleGames.size() != 0 && std::find(std::begin(requiredVersion.CompatibleGames), std::end(requiredVersion.CompatibleGames), FrameworkInstance->CurrentTyGame()) == std::end(requiredVersion.CompatibleGames)) {
            Logger::LogMessage(std::format("[Plugin Loader] {} Is Incompatible with Ty {}", pluginName, FrameworkInstance->CurrentTyGame()), Error);
            mPluginErrors.emplace(pluginName, std::format("Incompatible with Ty {}", FrameworkInstance->CurrentTyGame()));
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
        }

        //Check which major version is needed
        if (requiredVersion.Major != TygerFrameworkPluginVersion_Major) {
            Logger::LogMessage(std::format("[Plugin Loader] {} Requires TygerFramework Major Version {}.{}.{}, But Version {}.{}.{} is Installed", 
                                                       pluginName, 
                                                       requiredVersion.Major, requiredVersion.Minor, requiredVersion.Patch, //Plugin Required Version
                                                       TygerFrameworkPluginVersion_Major, TygerFrameworkPluginVersion_Minor, TygerFrameworkPluginVersion_Patch), Error); //Loader Version
            mPluginErrors.emplace(pluginName, std::format("Requires TygerFramework Major Version {}.{}.{}",
                                                           requiredVersion.Major, requiredVersion.Minor, requiredVersion.Patch));
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
        }
        //Check which minor and patch version is needed
        //Need to check the minor version for patch so that if the plugin needs a version like 1.1.2 and the loader is on 1.2.0 this doesn't give a false error
        else if (requiredVersion.Minor > TygerFrameworkPluginVersion_Minor || 
                (requiredVersion.Patch > TygerFrameworkPluginVersion_Patch && requiredVersion.Minor == TygerFrameworkPluginVersion_Minor)) {
            Logger::LogMessage(std::format("[Plugin Loader] {} Requires TygerFramework Version {}.{}.{} or Newer, But Version {}.{}.{} is Installed", 
                                                       pluginName, 
                                                       requiredVersion.Major, requiredVersion.Minor, requiredVersion.Patch, //Plugin Required Version
                                                       TygerFrameworkPluginVersion_Major, TygerFrameworkPluginVersion_Minor, TygerFrameworkPluginVersion_Patch), Error); //Loader Version
            mPluginErrors.emplace(pluginName, std::format("Requires TygerFramework Version {}.{}.{} or Newer",
                                                           requiredVersion.Major, requiredVersion.Minor, requiredVersion.Patch));
            FreeLibrary(pluginModule);
            plugins = mPlugins.erase(plugins);
            continue;
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
        Logger::LogMessage("[Plugin Loader] Initializing: " + pluginName);
        try {
            if (!pluginInitializer(&pluginInitParam)) {
                if (pluginInitParam.initErrorMessage != "")
                {
                    Logger::LogMessage("[Plugin Loader] Failed to Initialize: " + pluginName + ", With the Error: " + pluginInitParam.initErrorMessage, Error);
                    mPluginErrors.emplace(pluginName, "Failed to Initialize: " + pluginInitParam.initErrorMessage);
                }
                else
                {
                    Logger::LogMessage("[Plugin Loader] Failed to Initialize: " + pluginName + ", With No Error Message Provided", Error);
                    mPluginErrors.emplace(pluginName, "Failed to Initialize: No Error Message Provided");
                }
                FreeLibrary(pluginModule);
                plugins = mPlugins.erase(plugins);
                continue;
            }
        }
        catch (...) {
            Logger::LogMessage("[Plugin Loader] " + pluginName + "Had An Exception Occur In TygerFrameworkPluginInitialize, Skipping", Error);
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

        ImGui::Spacing();

        if (!mDependencies.empty()) {
            ImGui::Text("Loaded Dependencies:");
            ImGui::TreePush("Dependencies");
            for (auto&& [name, _] : mDependencies) {
                ImGui::Text(name.c_str());
            }

            ImGui::TreePop();
        }

        if (!mDependencyErrors.empty()) {
            ImGui::Text("Dependency Errors:");
            ImGui::TreePush("Dependency Errors");
            for (auto&& [name, _] : mDependencyErrors) {
                ImGui::Text(name.c_str());
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
                    Logger::LogMessage("[" + pluginName + "]" + " Error, missing text for CollapsingHeader TygerFramework ImGui function! Returning Early", Error);
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
                    Logger::LogMessage("[" + pluginName + "]" + " Error, missing text for SetTooltip TygerFramework ImGui function! Skipping", Error);
                    break;
                }

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(param.Text.c_str());
                break;
            case TreePush:
                //Skip it if the text is blank
                if (param.Text == "") {
                    Logger::LogMessage("[" + pluginName + "]" + " Error, missing text for TreePush TygerFramework ImGui function! Skipping", Error);
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

bool PluginSetTyInputStateProxy(std::string pluginName, TyInputsFlags flags) {
    return FrameworkInstance->PluginLoader.SetTyInputState(pluginName, flags);
}

TyInputsFlags GetPluginsTyInputState(std::string pluginName)
{
    return FrameworkInstance->PluginLoader.GetPluginTyInputFlags()[pluginName];
}

//Blocker name is the one who wants to block input, for plugins it'll be the plugin name, but TygerFramework also uses this
bool PluginLoader::SetTyInputState(std::string blockerName, TyInputsFlags flags)
{
    mPluginTyInputFlags.insert_or_assign(blockerName, flags);
    CombineTyInputState();
    return true;
}

void PluginLoader::CombineTyInputState()
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
