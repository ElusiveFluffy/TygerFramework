#pragma once
#include <map>
#include <string>
#include "API.hpp"

bool TygerFrameworkDrawPluginUi(DrawUIFunc func);
bool TygerFrameworkPluginImguiHasFocus(ImGuiHasFocusFunc func);
bool TygerFrameworkPluginWndProc(WndProcFunc func);
HWND TygerFrameworkGetTyWindowHandle();

class PluginLoader
{
public:
	void EarlyInit();
	void Initialize();
	void DrawUI();

private:
	std::map<std::string, HMODULE> mPlugins{};
};

