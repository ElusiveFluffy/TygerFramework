#pragma once
#include <map>
#include <string>
#include "API.hpp"

bool TygerFrameworkDrawPluginUi(DrawUIFunc func);
bool TygerFrameworkPluginImguiHasFocus(ImGuiHasFocusFunc func);
bool TygerFrameworkPluginWndProc(WndProcFunc func);
HWND TygerFrameworkGetTyWindowHandle();
void TygerFrameworkSetImGuiFont(void* fonts);
void PluginSetTygerFrameworkImGuiElements(std::string pluginName, std::vector<TygerFrameworkImGuiParam> param);

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
};

