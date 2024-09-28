#pragma once
#include <filesystem>
#include <fstream>
#include "PluginLoader.h"
class TygerFramework
{
public:
	enum LogLevel
	{
		Info,
		Warning,
		Error
	};

	PluginLoader PluginLoader{};
	static std::filesystem::path GetPluginDir();
	auto getFrameworkModule() const { return mTyHModule; };
	int CurrentTyGame() const { return TyGame; };
	TygerFramework(HMODULE TyHModule);
	void LogMessage(std::string message, LogLevel errorType = Info);
	void Shutdown();

	void ToggleConsoleVisibility();

	//UI Settings
	bool ShowConsole;
	bool TyLogInConsole;
	bool RememberVisibility = true;
	bool InputPassthrough = true;

	HWND TyWindowHandle;

private:
	HMODULE mTyHModule;
	std::ofstream mLogger;

	int TyGame;
	static constexpr int Ty1AppID = 411960;
	static constexpr int Ty2AppID = 411970;
	static constexpr int Ty3AppID = 411980;
	void AttemptToDetectGameFromExe();
	void CreateConsole();
	void SaveSettings();
	void LoadSettings();

	bool HookTyDebugOutput();
};

extern std::unique_ptr<TygerFramework> FrameworkInstance;