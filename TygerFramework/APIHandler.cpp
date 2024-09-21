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
		try {
			function();
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] Error occured when running plugin draw UI");
		}
	}
}

//Subscribe plugin functions to the Draw UI loop
bool APIHandler::AddDrawPluginUIFunc(std::function<void()> func)
{
	mDrawPluginUIFunctions.push_back(func);
	return true;
}

bool APIHandler::PluginImGuiHasFocus()
{
	bool anyTrue = false;
	for (auto&& function : mPluginImGuiHasFocusFunctions) {
		try {
			if (function()) {
				anyTrue = true;
			}
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] Error occured while checking plugin imgui focus");
		}
	}
	return anyTrue;
}

bool APIHandler::AddPluginImGuiHasFocusFunc(std::function<bool()> func)
{
	mPluginImGuiHasFocusFunctions.push_back(func);
	return true;
}

//Returns true if any plugins want to block any WndProc
bool APIHandler::PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool anyTrue = false;
	for (auto&& function : mPluginWndProcFunctions) {
		try {
			if (function(hWnd, msg, wParam, lParam)) {
				anyTrue = true;
			}
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] Error occured when running plugin WndProc");
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

void APIHandler::TickBeforeGame(float deltaSeconds)
{
	for (auto&& function : mTickBeforeGameFunctions) {
		try {
			function(deltaSeconds);
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] Error occured when running plugin tick before game");
		}
	}
}

void APIHandler::AddTickBeforeGameFunc(std::function<void(float deltaSeconds)> func)
{
	mTickBeforeGameFunctions.push_back(func);
}
