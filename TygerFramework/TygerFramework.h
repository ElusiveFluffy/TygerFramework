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
	int WhichTyGame();
	TygerFramework(HMODULE tygerFrameworkModule);
	void LogMessage(std::string message, LogLevel errorType = Info);
private:
	HMODULE mTygerFrameworkModule;
	std::ofstream mLogger;

	int TyGame;
	static constexpr int Ty1AppID = 411960;
	static constexpr int Ty2AppID = 411970;
	static constexpr int Ty3AppID = 411980;
	void AttemptToDetectGameFromExe();
};

extern std::unique_ptr<TygerFramework> FrameworkInstance;