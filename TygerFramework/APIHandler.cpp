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
	for (auto&& params : mDrawPluginUIParams) {
		try {
			params.Function();
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] " + params.PluginName + " had an error occur when running plugin draw UI", TygerFramework::Error);
		}
	}
}

//Subscribe plugin functions to the Draw UI loop
bool APIHandler::AddDrawPluginUIFunc(DrawPluginUIParam param)
{
	mDrawPluginUIParams.push_back(param);
	return true;
}

bool APIHandler::PluginImGuiHasFocus()
{
	bool anyTrue = false;
	for (auto&& params : mPluginImGuiHasFocusParams) {
		try {
			if (params.Function()) {
				anyTrue = true;
			}
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] " + params.PluginName + " had an error occur while checking plugin imgui focus", TygerFramework::Error);
		}
	}
	return anyTrue;
}

bool APIHandler::AddPluginImGuiHasFocusFunc(PluginImGuiHasFocusParam param)
{
	mPluginImGuiHasFocusParams.push_back(param);
	return true;
}

//Returns true if any plugins want to block any WndProc
bool APIHandler::PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool anyTrue = false;
	for (auto&& params : mPluginWndProcParams) {
		try {
			if (params.Function(hWnd, msg, wParam, lParam)) {
				anyTrue = true;
			}
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] " + params.PluginName + " had an error occur when running plugin WndProc", TygerFramework::Error);
		}
	}
	return anyTrue;
}

//Subscribe plugin functions to the WndProc loop
bool APIHandler::AddPluginWndProcFunc(PluginWndProcParam param)
{
	mPluginWndProcParams.push_back(param);
	return true;
}

void APIHandler::TickBeforeGame(float deltaSeconds)
{
	for (auto&& params : mTickBeforeGameParams) {
		try {
			params.Function(deltaSeconds);
		}
		catch (...) {
			FrameworkInstance->LogMessage("[API Handler] " + params.PluginName + " had an error occur when running plugin tick before game", TygerFramework::Error);
		}
	}
}

void APIHandler::AddTickBeforeGameFunc(TickBeforeGameParam param)
{
	mTickBeforeGameParams.push_back(param);
}
