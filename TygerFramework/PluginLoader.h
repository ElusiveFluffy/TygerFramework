#pragma once
#include <map>
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
bool PluginSetTyBlockedInputProxy(std::string pluginName, TyBlockedInputsFlags flags);
TyBlockedInputsFlags GetPluginsTyBlockedInputState(std::string pluginName);

class PluginLoader
{
public:
	void EarlyInit();
	void Initialize();
	void DrawUI();
	void PluginDrawInTygerFrameworkWindow();
	std::map<std::string, std::vector<TygerFrameworkImGuiParam>> PluginImGuiElements{};
	std::map<std::string, std::string> mPluginWarnings{};

	std::map<std::string, TyBlockedInputsFlags>& GetPluginTyInputFlags() { return mPluginTyInputFlags; };
	bool SetTyBlockedInputs(std::string blockerName, TyBlockedInputsFlags flags);
	TyBlockedInputsFlags TyInputCombinedFlags = None;

private:
	void CombineTyBlockedInputs();

	std::map<std::string, HMODULE> mPlugins{};
	std::map<std::string, std::string> mPluginErrors{};
	//Just so each plugin can be kept track of without needing to have a check every frame for what input they want
	std::map<std::string, TyBlockedInputsFlags> mPluginTyInputFlags{};
};

