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

class PluginLoader
{
public:
	void EarlyInit();
	void Initialize();
	void DrawUI();
	void PluginDrawInTygerFrameworkWindow();
	std::map<std::string, std::vector<TygerFrameworkImGuiParam>> PluginImGuiElements{};
	std::map<std::string, std::string> mPluginWarnings{};

private:
	std::map<std::string, HMODULE> mPlugins{};
	std::map<std::string, std::string> mPluginErrors{};
};

