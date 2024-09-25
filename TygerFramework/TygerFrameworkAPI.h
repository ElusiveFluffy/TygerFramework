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
	std::vector<int> CompatibleGames; //Optional (List all the game numbers the plugin is compatible with (1 = Ty 1, 2 = Ty 2, 3 = Ty 3). Leave unchanged if it supports any game)
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
typedef void (*TickBeforeGameFunc)(float);

typedef bool (*TyFDrawPluginUI)(std::string, DrawUIFunc);
typedef bool (*TyFPluginImGuiHasFocus)(std::string, ImGuiHasFocusFunc);
typedef bool (*TyFPluginWndProc)(std::string, WndProcFunc);
typedef bool (*TyFTickBeforeGame)(std::string, TickBeforeGameFunc);

typedef struct {
	void (*LogPluginMessage)(std::string message, LogLevel logLevel);
	int (*CurrentTyGame)();
	TyFDrawPluginUI AddDrawPluginUI;
	TyFPluginImGuiHasFocus AddPluginImGuiHasFocus;
	TyFPluginWndProc AddPluginWndProc;
	HWND(*GetTyWindowHandle)();
	bool (*DrawingGUI)(); //Only use for imgui if you have a special use case
	void (*SetImGuiFont)(void* imguiFont);
	void (*SetTyFImGuiElements)(std::string pluginName, std::vector<TygerFrameworkImGuiParam> params);
	TyFTickBeforeGame AddTickBeforeGame;
}TygerFrameworkPluginFunctions;

typedef struct {
	void* tygerFrameworkModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
	std::string initErrorMessage; //Error message that gets read by TygerFramework if the plugin can't initialize (returning false on initialize)
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);
typedef void (*TyFPluginRequiredVersion)(TygerFrameworkPluginVersion*);