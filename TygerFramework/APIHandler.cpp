#include "framework.h"
#include "APIHandler.h"
#include "TygerFramework.h"
#include "Logger.h"

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
		catch (const std::exception& e) {
			std::string message = "[API Handler] " + params.PluginName + " had an error occur when running plugin draw UI: ";
			message += e.what();
			Logger::LogMessage(message, Error);
		}
		catch (...) {
			Logger::LogMessage("[API Handler] " + params.PluginName + " had an error occur when running plugin draw UI", Error);
		}
	}
}

//Subscribe plugin functions to the Draw UI loop
bool APIHandler::AddDrawPluginUIFunc(VoidFunctionParam param)
{
	mDrawPluginUIParams.push_back(param);
	return true;
}

bool APIHandler::PluginImGuiWantCaptureMouse()
{
	bool anyTrue = false;
	for (auto&& [pluginName, function] : mPluginImGuiWantCaptureMouseParams) {
		try {
			if (function()) {
				anyTrue = true;
			}
		}
		catch (const std::exception& e) {
			std::string message = "[API Handler] " + pluginName + " had an error occur while checking plugin imgui focus: ";
			message += e.what();
			Logger::LogMessage(message, Error);
		}
		catch (...) {
			Logger::LogMessage("[API Handler] " + pluginName + " had an error occur while checking plugin imgui focus", Error);
		}
	}
	return anyTrue;
}

bool APIHandler::AddPluginImGuiWantCaptureMouseFunc(PluginWantCaptureMouseParam param)
{
	//Replace the old function if its called a second time from the same plugin
	mPluginImGuiWantCaptureMouseParams.insert_or_assign(param.PluginName, param.Function);
	return true;
}

//Returns true if any plugins want to block any WndProc
bool APIHandler::PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool anyTrue = false;
	for (auto&& params : mPluginWndProcParams) {
		try {
			bool wantMouseCapture = false;
			//Check if the plugin wants to capture the mouse
			if (mPluginImGuiWantCaptureMouseParams.contains(params.PluginName))
				wantMouseCapture = mPluginImGuiWantCaptureMouseParams[params.PluginName]();
			else
			{
				//Add a warning if it doesn't have the function
				if (!FrameworkInstance->PluginLoader.mPluginWarnings.contains(params.PluginName))
				{
					FrameworkInstance->PluginLoader.mPluginWarnings.emplace(params.PluginName, "No ImGui WantMouseCapture API funciton");
					Logger::LogMessage("[API Handler] " + params.PluginName + " doesn't impliment the imgui WantMouseCapture API function", Warning);
				}
			}
			//Use if the plugin wants to capture the mouse here to fix a bug with the resizing cursor change
			//Don't want to run set cursor with the ImGui WndProc as it glitches the resize cursor on the TygerFramework window and every plugin window except the last one it gets ran on
			if (wantMouseCapture || msg != WM_SETCURSOR)
				if (params.Function(hWnd, msg, wParam, lParam)) {
					anyTrue = true;
				}
		}
		catch (const std::exception& e) {
			std::string message = "[API Handler] " + params.PluginName + " had an error occur when running plugin WndProc: ";
			message += e.what();
			Logger::LogMessage(message, Error);
		}
		catch (...) {
			Logger::LogMessage("[API Handler] " + params.PluginName + " had an error occur when running plugin WndProc", Error);
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
		catch (const std::exception& e) {
			std::string message = "[API Handler] " + params.PluginName + " had an error occur when running plugin tick before game: ";
			message += e.what();
			Logger::LogMessage(message, Error);
		}
		catch (...) {
			Logger::LogMessage("[API Handler] " + params.PluginName + " had an error occur when running plugin tick before game", Error);
		}
	}
}

void APIHandler::AddTickBeforeGameFunc(TickBeforeGameParam param)
{
	mTickBeforeGameParams.push_back(param);
}

void APIHandler::OnTyInitialized()
{
	for (auto&& params : mOnTyInitializedParams) {
		try {
			params.Function();
		}
		catch (const std::exception& e) {
			std::string message = "[API Handler] Had an error occur when notifying " + params.PluginName + " that Ty had initialized: ";
			message += e.what();
			Logger::LogMessage(message, Error);
		}
		catch (...) {
			Logger::LogMessage("[API Handler] Had an error occur when notifying " + params.PluginName + " that Ty had initialized", Error);
		}
	}
}

void APIHandler::AddOnTyInitializedFunc(VoidFunctionParam param)
{
	mOnTyInitializedParams.push_back(param);
}

void APIHandler::OnTyBeginShutdown()
{
	for (auto&& params : mOnTyBeginShutdownParams) {
		try {
			params.Function();
		}
		catch (const std::exception& e) {
			std::string message = "[API Handler] Had an error occur when notifying " + params.PluginName + " that Ty had begun shutting down: ";
			message += e.what();
			Logger::LogMessage(message, Error);
		}
		catch (...) {
			Logger::LogMessage("[API Handler] Had an error occur when notifying " + params.PluginName + " that Ty had begun shutting down", Error);
		}
	}
}

void APIHandler::AddOnTyBeginShutdownFunc(VoidFunctionParam param)
{
	mOnTyBeginShutdownParams.push_back(param);
}
