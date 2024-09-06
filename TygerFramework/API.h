#pragma once
#include <string>

enum LogLevel
{
	Info,
	Warning,
	Error
};

typedef void (*DrawUIFunc)();
typedef bool (*WndProcFunc)(HWND, UINT, WPARAM, LPARAM);

typedef bool (*TyFDrawPluginUI)(DrawUIFunc);
typedef bool (*TyFPluginWndProc)(WndProcFunc);

typedef struct {
	void (*LogPluginMessage)(std::string message, LogLevel logLevel);
	int (*WhichTyGame)();
	TyFDrawPluginUI DrawPluginUI;
	TyFPluginWndProc PluginWndProc;
	void* (*GetImGuiContext)();
}TygerFrameworkPluginFunctions;

typedef struct {
	void* tygerFrameworkModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);