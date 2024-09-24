#pragma once
#include <map>
#include <string>
#include "TygerFrameworkAPI.hpp"

bool TygerFrameworkDrawPluginUi(std::string pluginName, DrawUIFunc func);
bool TygerFrameworkPluginImguiHasFocus(std::string pluginName, ImGuiHasFocusFunc func);
bool TygerFrameworkPluginWndProc(std::string pluginName, WndProcFunc func);
HWND TygerFrameworkGetTyWindowHandle();
void TygerFrameworkSetImGuiFont(void* fonts);
void PluginSetTygerFrameworkImGuiElements(std::string pluginName, std::vector<TygerFrameworkImGuiParam> param);
bool TygerFrameworkTickBeforeGame(std::string pluginName, TickBeforeGameFunc func);

class PluginLoader
{
public:
	void EarlyInit();
	void Initialize();
	void DrawUI();
	void PluginDrawInTygerFrameworkWindow();
	std::map<std::string, std::vector<TygerFrameworkImGuiParam>> PluginImGuiElements{};

private:
	std::map<std::string, HMODULE> mPlugins{};
	std::map<std::string, std::string> mPluginErrors{};
	std::map<std::string, std::string> mPluginWarnings{};
};

