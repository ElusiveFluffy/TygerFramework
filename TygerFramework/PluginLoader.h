#pragma once
#include <map>
#include <unordered_map>
#include <string>
#include "TygerFrameworkAPI.hpp"

bool TygerFrameworkDrawPluginUi(std::string pluginName, VoidFunc func);
bool TygerFrameworkPluginImguiWantCaptureMouse(std::string pluginName, ImGuiWantCaptureMouseFunc func);
bool TygerFrameworkPluginWndProc(std::string pluginName, WndProcFunc func);
HWND TygerFrameworkGetTyWindowHandle();
bool TygerFrameworkDrawingGUI();
void TygerFrameworkSetImGuiFont(void* fonts);
void PluginSetTygerFrameworkImGuiElements(std::string pluginName, std::vector<TygerFrameworkImGuiParam> param);
bool TygerFrameworkTickBeforeGame(std::string pluginName, TickBeforeGameFunc func);
bool TygerFrameworkOnTyInitialized(std::string pluginName, VoidFunc func);
bool TygerFrameworkOnTyBeginShutdown(std::string pluginName, VoidFunc func);
bool PluginSetTyInputStateProxy(std::string pluginName, TyInputsFlags flags);
TyInputsFlags GetPluginsTyInputState(std::string pluginName);

class PluginLoader
{
public:
	void EarlyInit();
	void Initialize();
	void DrawUI();
	void PluginDrawInTygerFrameworkWindow();
	std::map<std::string, std::vector<TygerFrameworkImGuiParam>> PluginImGuiElements{};
	std::map<std::string, std::string> mPluginWarnings{};

	std::unordered_map<std::string, TyInputsFlags>& GetPluginTyInputFlags() { return mPluginTyInputFlags; };
	bool SetTyInputState(std::string blockerName, TyInputsFlags flags);
	TyInputsFlags TyInputCombinedFlags = None;

private:
	void CombineTyInputState();
	void PluginEarlyInit();
	void DependencyInit();

	std::map<std::string, HMODULE> mPlugins{};
	std::map<std::string, std::string> mPluginErrors{};

	std::map<std::string, HMODULE> mDependencies{};
	std::map<std::string, std::string> mDependencyErrors{};
	//Just so each plugin can be kept track of without needing to have a check every frame for what input they want
	std::unordered_map<std::string, TyInputsFlags> mPluginTyInputFlags{};
};

