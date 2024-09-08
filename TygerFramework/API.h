#pragma once
#include <string>
#include <vector>

enum LogLevel
{
	Info,
	Warning,
	Error
};

//Elements to render stuff in the TygerFramework ImGui window
//Limited amount of elements as this is only for minimal stuff
//(This is the main way I could think of to do it without ImGui version incompatibilities)
enum TyFImGuiElements {
	CollapsingHeader, //Needs text for header name
	Text, //Needs text
	SameLine, //No text
	SetTooltip, //Needs text for tooltip (Adds a tooltip to the previous element)
	TreePush, //Needs texts for tree name
	TreePop //No text (Make sure to call when done after using TreePush)
};

typedef struct {
	TyFImGuiElements ImGuiElement;
	std::string Text;
}TygerFrameworkImGuiParam;

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
	void (*SetTyFImGuiElements)(std::string pluginName, std::vector<TygerFrameworkImGuiParam> params);
}TygerFrameworkPluginFunctions;

typedef struct {
	void* tygerFrameworkModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);