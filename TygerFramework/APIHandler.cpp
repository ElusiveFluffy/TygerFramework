#include "framework.h"
#include "APIHandler.h"
#include "TygerFramework.h"

std::shared_ptr<APIHandler>& APIHandler::Get()
{
	static auto instance = std::make_shared<APIHandler>();
	return instance;
}

//Run all the UI code for subscribed plugins
void APIHandler::DrawPluginUI()
{
	for (auto&& function : mDrawPluginUIFunctions) {
		function();
	}
}

//Subscribe plugin functions to the Draw UI loop
bool APIHandler::AddDrawPluginUIFunc(std::function<void()> func)
{
	mDrawPluginUIFunctions.push_back(func);
	return true;
}

//Returns true if any plugins want to block any WndProc
bool APIHandler::PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool anyTrue = false;
	for (auto&& function : mPluginWndProcFunctions) {
		if (function(hWnd, msg, wParam, lParam)) {
			anyTrue = true;
		}
	}
	return anyTrue;
}

//Subscribe plugin functions to the WndProc loop
bool APIHandler::AddPluginWndProcFunc(TyFPluginWndProc func)
{
	mPluginWndProcFunctions.push_back(func);
	return true;
}
