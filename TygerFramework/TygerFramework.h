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
	static std::filesystem::path GetDependencyDir();
	auto getFrameworkModule() const { return mTyHModule; };
	int CurrentTyGame() const { return TyGame; };
	TygerFramework(HMODULE TyHModule);
	void LogMessage(std::string message, LogLevel errorType = Info);
	void Shutdown();

	bool SetTyInputFlag(TyInputsFlags flag, bool enableFlag);

	void ToggleConsoleVisibility();

	void CheckIfGameFinishInit();
	bool TyHasInitialized;
	bool CursorShown;

	//UI Settings
	bool ShowConsole;
	bool TyLogInConsole;
	bool RememberVisibility = true;
	bool InputPassthrough = true;
	bool KeyboardAlwaysPassthrough;

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
	bool HookTyShutdown();
};

extern std::unique_ptr<TygerFramework> FrameworkInstance;