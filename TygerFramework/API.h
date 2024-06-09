#pragma once
#include <string>

enum LogLevel
{
	Info,
	Warning,
	Error
};

typedef struct {
	void (*LogPluginMessage)(std::string message, LogLevel logLevel);
	int (*WhichTyGame)();
}TygerFrameworkPluginFunctions;

typedef struct {
	void* tygerFrameworkModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);