#pragma once
#include <string>
#include <vector>
#include <filesystem>

//Needed for the WndProc inputs usually in precompiled headers but that needs to be turned off for imgui
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

constexpr int TygerFrameworkPluginVersion_Major = 1;
constexpr int TygerFrameworkPluginVersion_Minor = 1;
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

//Flags to block Ty from receiving inputs like mouse clicks
enum TyInputsFlags {
	None = 0,
	NoMouseClickInput = 1 << 0,
	NoMouseCameraInput = 1 << 1,
	NoKeyboardInput = 1 << 2,
	TyShowCursor = 1 << 3,

	NoMouseInput = NoMouseClickInput | NoMouseCameraInput
};
DEFINE_ENUM_FLAG_OPERATORS(TyInputsFlags)

typedef struct {
	TyFImGuiElements ImGuiElement;
	std::string Text;
}TygerFrameworkImGuiParam;

typedef void (*VoidFunc)();
typedef bool (*ImGuiWantCaptureMouseFunc)();
typedef bool (*WndProcFunc)(HWND, UINT, WPARAM, LPARAM);
typedef void (*TickBeforeGameFunc)(float);

typedef bool (*TyFDrawPluginUI)(std::string, VoidFunc);
typedef bool (*TyFPluginImGuiWantCaptureMouse)(std::string, ImGuiWantCaptureMouseFunc);
typedef bool (*TyFPluginWndProc)(std::string, WndProcFunc);
typedef bool (*TyFTickBeforeGame)(std::string, TickBeforeGameFunc);

//Order of these matters to be backwards compatible
typedef struct {
	//v0.1
	void (*LogPluginMessage)(std::string message, LogLevel logLevel);
	int (*CurrentTyGame)();

	//v1.0.0
	TyFDrawPluginUI AddDrawPluginUI;
	TyFPluginImGuiWantCaptureMouse AddPluginImGuiWantCaptureMouse;
	TyFPluginWndProc AddPluginWndProc;
	HWND(*GetTyWindowHandle)();
	bool (*DrawingGUI)();
	void (*SetImGuiFont)(void* imguiFont);
	void (*SetTyFImGuiElements)(std::string pluginName, std::vector<TygerFrameworkImGuiParam> params);
	TyFTickBeforeGame AddTickBeforeGame;

	//v1.1.0
	bool (*AddOnTyInitialized)(std::string, VoidFunc);
	bool (*AddOnTyBeginShutdown)(std::string, VoidFunc);
	bool (*SetTyInputState)(std::string pluginName, TyInputsFlags flags);
	TyInputsFlags (*GetTyInputState)(std::string pluginName);
	std::filesystem::path (*GetPluginDir)();
}TygerFrameworkPluginFunctions;

//Order of these matters to be backwards compatible
typedef struct {
	void* TyHModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
	std::string initErrorMessage; //Error message that gets read by TygerFramework if the plugin can't initialize (returning false on initialize)
}TygerFrameworkPluginInitializeParam;

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);
typedef void (*TyFPluginRequiredVersion)(TygerFrameworkPluginVersion*);