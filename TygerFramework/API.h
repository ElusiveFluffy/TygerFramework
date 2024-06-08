#pragma once
#include <string>

enum LogLevel
{
	Info,
	Warning,
	Error
};

typedef struct {
	void (*LogPluginMessage)(std::string message);
	void (*LogPluginProblem)(std::string message, LogLevel logLevel);
	int (*WhichTyGame)();
}TygerFrameworkPluginFunctions;

typedef struct {
	void* tygerFrameworkModule;
	const TygerFrameworkPluginFunctions* functions;
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);