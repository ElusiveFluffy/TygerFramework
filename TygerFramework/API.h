#pragma once
#include <string>

enum LogLevel
{
	Info,
	Warning,
	Error
};

typedef void (*DrawUIFunc)();
typedef bool (*ImGuiHasFocusFunc)();
typedef bool (*WndProcFunc)(HWND, UINT, WPARAM, LPARAM);

typedef bool (*TyFDrawPluginUI)(DrawUIFunc);
typedef bool (*TyFPluginImGuiHasFocus)(ImGuiHasFocusFunc);
typedef bool (*TyFPluginWndProc)(WndProcFunc);

typedef struct {
	void (*LogPluginMessage)(std::string message, LogLevel logLevel);
	int (*WhichTyGame)();
	TyFDrawPluginUI DrawPluginUI;
	TyFPluginImGuiHasFocus PluginImGuiHasFocus;
	TyFPluginWndProc PluginWndProc;
	HWND(*GetTyWindowHandle)();
	void (*SetImGuiFont)(void* imguiFont);
}TygerFrameworkPluginFunctions;

typedef struct {
	void* tygerFrameworkModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);