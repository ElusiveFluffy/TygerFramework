#pragma once
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <filesystem>

//Needed for the WndProc inputs usually in precompiled headers but that needs to be turned off for imgui
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

constexpr int TygerFrameworkPluginVersion_Major = 1;
constexpr int TygerFrameworkPluginVersion_Minor = 2;
constexpr int TygerFrameworkPluginVersion_Patch = 0;

struct TygerFrameworkPluginVersion {
	int Major;
	int Minor;
	int Patch;
	std::vector<int> CompatibleGames; //Optional (List all the game numbers the plugin is compatible with (1 = Ty 1, 2 = Ty 2, 3 = Ty 3). Leave unchanged if it supports any game)
};

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
	TreePop, //No text (Make sure to call when done after using TreePush)
	CollapsingHeaderEnd //No text (Not a real ImGUI element but it is needed for this)
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

struct TygerFrameworkImGuiParam {
	TyFImGuiElements ImGuiElement;
	std::string Text;
};

typedef void (*VoidFunc)();
typedef bool (*ImGuiWantCaptureMouseFunc)();
typedef bool (*WndProcFunc)(HWND, UINT, WPARAM, LPARAM);
typedef void (*TickBeforeGameFunc)(float);

typedef bool (*TyFDrawPluginUI)(std::string, VoidFunc);
typedef bool (*TyFPluginImGuiWantCaptureMouse)(std::string, ImGuiWantCaptureMouseFunc);
typedef bool (*TyFPluginWndProc)(std::string, WndProcFunc);
typedef bool (*TyFTickBeforeGame)(std::string, TickBeforeGameFunc);

//Order of these matters to be backwards compatible
struct TygerFrameworkPluginFunctions {
	//v0.1
	void (*LogPluginMessage)(std::string message, LogLevel logLevel);
	int (*CurrentTyGame)();

	//v1.0.0
	TyFDrawPluginUI AddDrawPluginUI;
	TyFPluginImGuiWantCaptureMouse AddPluginImGuiWantCaptureMouse;
	TyFPluginWndProc AddPluginWndProc;
	HWND(*GetTyWindowHandle)();
	bool (*DrawingGUI)();
	// Deprecated, don't use
	void (*SetImGuiFont)(void* imguiFont);
	void (*SetTyFImGuiElements)(std::string pluginName, std::vector<TygerFrameworkImGuiParam> params);
	TyFTickBeforeGame AddTickBeforeGame;

	//v1.1.0
	bool (*AddOnTyInitialized)(std::string, VoidFunc);
	bool (*AddOnTyBeginShutdown)(std::string, VoidFunc);
	bool (*SetTyInputState)(std::string pluginName, TyInputsFlags flags);
	TyInputsFlags(*GetTyInputState)(std::string pluginName);
	std::filesystem::path(*GetPluginDir)();
};

//Order of these matters to be backwards compatible
struct TygerFrameworkPluginInitializeParam {
	void* TyHModule;
	std::string pluginFileName;
	const TygerFrameworkPluginFunctions* functions;
	std::string initErrorMessage; //Error message that gets read by TygerFramework if the plugin can't initialize (returning false on initialize)
};

typedef bool (*TyFPluginInitializer)(const TygerFrameworkPluginInitializeParam*);
typedef void (*TyFPluginRequiredVersion)(TygerFrameworkPluginVersion*);

class API {
public:
	//Will default to the plugin's file name on initialization, the name will be used in the logs
	//If you want a different name, can change the plugin name in DllMain
	std::string PluginName = "Plugin";

	//Returns true if initialized
	static bool IsInitialized() {
		return mInstance != nullptr;
	}

	//Call this when the TygerFramework Plugin Initialize export function gets called, 
	//and before you call any other API functions.
	static auto& Initialize(const TygerFrameworkPluginInitializeParam* param) {
		if (param == nullptr) {
			throw std::runtime_error("param is null");
		}

		if (mInstance != nullptr) {
			throw std::runtime_error("API already initialized");
		}

		mInstance = std::make_unique<API>(param);

		//If the plugin name has already been changed to something custom it won't change it to the file name
		if (mInstance->PluginName == "Plugin")
			mInstance->PluginName = param->pluginFileName;

		return mInstance;
	}

	//Only call this AFTER calling initialize
	static API* Get() {
		if (mInstance == nullptr) {
			throw std::runtime_error("API not initialized");
		}

		return mInstance.get();
	}

	API(const TygerFrameworkPluginInitializeParam* param)
		: mParam{ param }
	{
	}

	inline const auto param() const {
		return mParam;
	}

	//Gets the current Ty window handle
	static HWND GetTyWindowHandle() {
		return API::Get()->param()->functions->GetTyWindowHandle();
	}

	//Checks if TygerFramework is drawing the GUI
	static bool DrawingGUI() {
		return API::Get()->param()->functions->DrawingGUI();
	}

	/// <summary>
	/// Gets the currently running Ty Game
	/// </summary>
	/// <returns>0: Couldn't detect which game
	/// <para>1: Ty 1</para>
	/// <para>2: Ty 2</para>
	/// <para>3: Ty 3</para></returns>
	static int CurrentTyGame() {
		return Get()->param()->functions->CurrentTyGame();
	}

	//Gets the current plugin directory (will be different for debug/release builds of TygerFramework)
	static std::filesystem::path GetPluginDirectory() {
		return Get()->param()->functions->GetPluginDir();
	}

	//Writes a message to the console and the log file. Default log level is info
	static void LogPluginMessage(std::string message, LogLevel logLevel = Info) {
		Get()->param()->functions->LogPluginMessage("[" + mInstance->PluginName + "] " + message, logLevel);
	}

	//Sets the elements from the plugin that will be drawn below the plugin section in the TygerFramework ImGui window.
	//Overwrites the old value if its called again
	static void SetTygerFrameworkImGuiElements(std::vector<TygerFrameworkImGuiParam> elements) {
		Get()->param()->functions->SetTyFImGuiElements(mInstance->PluginName, elements);
	}

	//Sets all the flags
	static bool SetTyInputState(TyInputsFlags flags) {
		return Get()->param()->functions->SetTyInputState(mInstance->PluginName, flags);
	}

	//More easily set or unset a flag(s) in some cases
	static bool SetTyInputFlag(TyInputsFlags flag, bool enableFlag) {
		if (enableFlag)
			return Get()->param()->functions->SetTyInputState(mInstance->PluginName, (GetTyInputState() | flag));
		else
			return Get()->param()->functions->SetTyInputState(mInstance->PluginName, (GetTyInputState() & ~flag));
	}

	//Get the input state of the game set by this plugin (the plugin state could still be blocked by another plugin though)
	static TyInputsFlags GetTyInputState() {
		return Get()->param()->functions->GetTyInputState(mInstance->PluginName);
	}

	//--------------------------
	//Event subscriber functions
	//--------------------------

	static bool AddDrawPluginUI(VoidFunc func) {
		return Get()->param()->functions->AddDrawPluginUI(mInstance->PluginName, func);
	}

	static bool AddPluginImGuiWantCaptureMouse(ImGuiWantCaptureMouseFunc func) {
		return Get()->param()->functions->AddPluginImGuiWantCaptureMouse(mInstance->PluginName, func);
	}

	static bool AddPluginWndProc(WndProcFunc func) {
		return Get()->param()->functions->AddPluginWndProc(mInstance->PluginName, func);
	}

	static bool AddTickBeforeGame(TickBeforeGameFunc func) {
		return Get()->param()->functions->AddTickBeforeGame(mInstance->PluginName, func);
	}

	static bool AddOnTyInitialized(VoidFunc func) {
		return Get()->param()->functions->AddOnTyInitialized(mInstance->PluginName, func);
	}

	static bool AddOnTyBeginShutdown(VoidFunc func) {
		return Get()->param()->functions->AddOnTyBeginShutdown(mInstance->PluginName, func);
	}

private:
	static inline std::unique_ptr<API> mInstance;
	const TygerFrameworkPluginInitializeParam* mParam;
};