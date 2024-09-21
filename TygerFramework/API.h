#pragma once
#include <string>
#include <vector>

constexpr int TygerFrameworkPluginVersion_Major = 1;
constexpr int TygerFrameworkPluginVersion_Minor = 0;
constexpr int TygerFrameworkPluginVersion_Patch = 0;

typedef struct {
	int Major;
	int Minor;
	int Patch;
	int GameNumber; //Optional (If the plugin is compatible with 2 or more games write it out like 12 for Ty 1 and 2. 13 for Ty 1, and 3. Etc. Leave unchanged for any game)
} TygerFrameworkPluginVersion;

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
	TextWrapped, //Needs text
	SameLine, //No text
	NewLine, //No text
	Spacing, //No text
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
typedef void (*TickBeforeGameFunc)(float deltaSeconds);

typedef bool (*TyFDrawPluginUI)(DrawUIFunc);
typedef bool (*TyFPluginImGuiHasFocus)(ImGuiHasFocusFunc);
typedef bool (*TyFPluginWndProc)(WndProcFunc);
typedef bool (*TyFTickBeforeGame)(TickBeforeGameFunc);

typedef struct {
	void (*LogPluginMessage)(std::string message, LogLevel logLevel);
	int (*WhichTyGame)();
	TyFDrawPluginUI AddDrawPluginUI;
	TyFPluginImGuiHasFocus AddPluginImGuiHasFocus;
	TyFPluginWndProc AddPluginWndProc;
	HWND(*GetTyWindowHandle)();
	void (*SetImGuiFont)(void* imguiFont);
	void (*SetTyFImGuiElements)(std::string pluginName, std::vector<TygerFrameworkImGuiParam> params);
	TyFTickBeforeGame AddTickBeforeGame;
}TygerFrameworkPluginFunctions;

typedef struct {
	void* tygerFrameworkModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);
typedef void (*TyFPluginRequiredVersion)(TygerFrameworkPluginVersion*);