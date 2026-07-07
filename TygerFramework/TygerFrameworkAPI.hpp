#pragma once
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <type_traits>

//Needed for the WndProc inputs usually in precompiled headers but that needs to be turned off for imgui
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

constexpr int TygerFrameworkPluginVersion_Major = 1;
constexpr int TygerFrameworkPluginVersion_Minor = 2;
constexpr int TygerFrameworkPluginVersion_Patch = 1;

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

/**
 * Optional C ABI surface (issue #2).
 *
 * POD/standard-layout mirror of the API. No STL types appear below, so plugins
 * built with a different toolchain/STL than TygerFramework stay binary-compatible
 * across the DLL boundary.
 */

/** C-ABI mirror of TygerFrameworkPluginVersion. */
struct TygerFrameworkPluginVersionC {
	int Major;             ///< Major version component.
	int Minor;             ///< Minor version component.
	int Patch;             ///< Patch version component.
	int CompatibleGames;   ///< Bitmask: bit (n-1) set => compatible with Ty n; 0 => any game.
};

/** C-ABI mirror of TygerFrameworkImGuiParam. */
struct TygerFrameworkImGuiParamC {
	int ImGuiElement;   ///< A TyFImGuiElements value.
	const char* Text;   ///< Borrowed only for the duration of the call.
};

/**
 * C-ABI mirror of the plugin function table.
 *
 * Populated by the framework and reached through TygerFrameworkPluginInitializeParamC.
 * The C++ API wrapper calls through these pointers, so plugin code normally uses the
 * API:: static methods rather than invoking them directly.
 */
struct TygerFrameworkPluginFunctionsC {
	void (*LogPluginMessage)(const char* message, int logLevel);
	int  (*CurrentTyGame)();
	bool (*AddDrawPluginUI)(const char* pluginName, VoidFunc func);
	bool (*AddPluginImGuiWantCaptureMouse)(const char* pluginName, ImGuiWantCaptureMouseFunc func);
	bool (*AddPluginWndProc)(const char* pluginName, WndProcFunc func);
	HWND (*GetTyWindowHandle)();
	bool (*DrawingGUI)();
	void (*SetTyFImGuiElements)(const char* pluginName, const TygerFrameworkImGuiParamC* params, int count);
	bool (*AddTickBeforeGame)(const char* pluginName, TickBeforeGameFunc func);
	bool (*AddOnTyInitialized)(const char* pluginName, VoidFunc func);
	bool (*AddOnTyBeginShutdown)(const char* pluginName, VoidFunc func);
	bool (*SetTyInputState)(const char* pluginName, int flags);   ///< flags is a TyInputsFlags value.
	int  (*GetTyInputState)(const char* pluginName);              ///< Returns a TyInputsFlags value.
	const char* (*GetPluginDir)();   ///< Framework-owned, stable for the plugin lifetime (UTF-8).
	bool (*CreateHook)(void* pTarget, void* pDetour, void** ppOriginal);
	bool (*DestroyHook)(void* pTarget);
};

/** C-ABI mirror of TygerFrameworkPluginInitializeParam, passed to the C init entry point. */
struct TygerFrameworkPluginInitializeParamC {
	void* TyHModule;                                   ///< Module handle of the running Ty game.
	const char* pluginFileName;                        ///< Plugin file name (framework-owned).
	const TygerFrameworkPluginFunctionsC* functions;   ///< The C function table.
};

static_assert(std::is_standard_layout_v<TygerFrameworkPluginVersionC>, "TygerFrameworkPluginVersionC must be standard-layout (C ABI)");
static_assert(std::is_standard_layout_v<TygerFrameworkImGuiParamC>, "TygerFrameworkImGuiParamC must be standard-layout (C ABI)");
static_assert(std::is_standard_layout_v<TygerFrameworkPluginFunctionsC>, "TygerFrameworkPluginFunctionsC must be standard-layout (C ABI)");
static_assert(std::is_standard_layout_v<TygerFrameworkPluginInitializeParamC>, "TygerFrameworkPluginInitializeParamC must be standard-layout (C ABI)");

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

	/**
	 * Framework-internal plumbing: binds the C function table.
	 *
	 * The TYGERFRAMEWORK_PLUGIN init shim calls this exactly once, before the
	 * author's handler runs. The Internal suffix marks it internal; plugin
	 * code must never call it directly (doing so would swap the active function
	 * table mid-run).
	 */
	static void BindCInterfaceInternal(const TygerFrameworkPluginInitializeParamC* paramC) {
		mCParam = paramC;
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
		return CFunctions()->GetTyWindowHandle();
	}

	//Checks if TygerFramework is drawing the GUI
	static bool DrawingGUI() {
		return CFunctions()->DrawingGUI();
	}

	/// <summary>
	/// Gets the currently running Ty Game
	/// </summary>
	/// <returns>0: Couldn't detect which game
	/// <para>1: Ty 1</para>
	/// <para>2: Ty 2</para>
	/// <para>3: Ty 3</para></returns>
	static int CurrentTyGame() {
		return CFunctions()->CurrentTyGame();
	}

	//Gets the current plugin directory (will be different for debug/release builds of TygerFramework)
	static std::filesystem::path GetPluginDirectory() {
		return std::filesystem::path(CFunctions()->GetPluginDir());
	}

	//Writes a message to the console and the log file. Default log level is info
	static void LogPluginMessage(std::string message, LogLevel logLevel = Info) {
		CFunctions()->LogPluginMessage(("[" + mInstance->PluginName + "] " + message).c_str(), (int)logLevel);
	}

	//Sets the elements from the plugin that will be drawn below the plugin section in the TygerFramework ImGui window.
	//Overwrites the old value if its called again
	static void SetTygerFrameworkImGuiElements(std::vector<TygerFrameworkImGuiParam> elements) {
		std::vector<TygerFrameworkImGuiParamC> c;
		c.reserve(elements.size());
		for (auto&& e : elements)
			c.push_back(TygerFrameworkImGuiParamC{ (int)e.ImGuiElement, e.Text.c_str() });
		CFunctions()->SetTyFImGuiElements(mInstance->PluginName.c_str(), c.data(), (int)c.size());
	}

	//Sets all the flags
	static bool SetTyInputState(TyInputsFlags flags) {
		return CFunctions()->SetTyInputState(mInstance->PluginName.c_str(), (int)flags);
	}

	//More easily set or unset a flag(s) in some cases
	static bool SetTyInputFlag(TyInputsFlags flag, bool enableFlag) {
		if (enableFlag)
			return CFunctions()->SetTyInputState(mInstance->PluginName.c_str(), (int)(GetTyInputState() | flag));
		else
			return CFunctions()->SetTyInputState(mInstance->PluginName.c_str(), (int)(GetTyInputState() & ~flag));
	}

	//Get the input state of the game set by this plugin (the plugin state could still be blocked by another plugin though)
	static TyInputsFlags GetTyInputState() {
		return (TyInputsFlags)CFunctions()->GetTyInputState(mInstance->PluginName.c_str());
	}

	//-----------------
	//Minhook functions
	//-----------------
	// Creates and enables a hook using MinHook.
	static bool CreateHook(void* pTarget, void* pDetour, void** ppOriginal) {
		if (!pTarget || !pDetour || !ppOriginal) return false;
		return CFunctions()->CreateHook(pTarget, pDetour, ppOriginal);
	}

	static bool DestroyHook(void* pTarget) {
		if (!pTarget) return false;
		return CFunctions()->DestroyHook(pTarget);
	}

	//--------------------------
	//Event subscriber functions
	//--------------------------

	static bool AddDrawPluginUI(VoidFunc func) {
		return CFunctions()->AddDrawPluginUI(mInstance->PluginName.c_str(), func);
	}

	static bool AddPluginImGuiWantCaptureMouse(ImGuiWantCaptureMouseFunc func) {
		return CFunctions()->AddPluginImGuiWantCaptureMouse(mInstance->PluginName.c_str(), func);
	}

	static bool AddPluginWndProc(WndProcFunc func) {
		return CFunctions()->AddPluginWndProc(mInstance->PluginName.c_str(), func);
	}

	static bool AddTickBeforeGame(TickBeforeGameFunc func) {
		return CFunctions()->AddTickBeforeGame(mInstance->PluginName.c_str(), func);
	}

	static bool AddOnTyInitialized(VoidFunc func) {
		return CFunctions()->AddOnTyInitialized(mInstance->PluginName.c_str(), func);
	}

	static bool AddOnTyBeginShutdown(VoidFunc func) {
		return CFunctions()->AddOnTyBeginShutdown(mInstance->PluginName.c_str(), func);
	}

private:
	static std::unique_ptr<API> mInstance;
	const TygerFrameworkPluginInitializeParam* mParam;
	static const TygerFrameworkPluginInitializeParamC* mCParam;   ///< Bound C interface (see BindCInterfaceInternal).
	/** Returns the bound C function table; throws if the API is not initialized or the C interface is not bound. */
	static const TygerFrameworkPluginFunctionsC* CFunctions() {
		if (mInstance == nullptr)
			throw std::runtime_error("API not initialized");
		if (mCParam == nullptr || mCParam->functions == nullptr)
			throw std::runtime_error("TygerFramework C interface not bound (use the TYGERFRAMEWORK_PLUGIN macro)");
		return mCParam->functions;
	}
};

inline std::unique_ptr<API> API::mInstance;
inline const TygerFrameworkPluginInitializeParamC* API::mCParam = nullptr;

/**
 * Plugin entry-point macro. Emits the exported extern "C" POD entry points the
 * framework calls, marshalling between the C wire types and the author's C++
 * handlers. Use EXACTLY ONCE in a plugin. Handlers stay pure C++:
 *   versionFn:  void(TygerFrameworkPluginVersion&)
 *   initFn:     bool(const TygerFrameworkPluginInitializeParam*)
 *
 * The emitted TygerFrameworkPluginInitializeC is called once per plugin load. It
 * uses function-local statics for the C++ init param and the error string so they
 * remain valid after the shim returns (the framework reads/copies them
 * immediately). Inside initFn the param's functions field is intentionally null --
 * use the API:: static methods (which call through the bound C table), never
 * param->functions.
 */
#define TYGERFRAMEWORK_PLUGIN(versionFn, initFn)                                                       \
	extern "C" __declspec(dllexport)                                                                   \
	void TygerFrameworkPluginRequiredVersionC(TygerFrameworkPluginVersionC* out) {                     \
		TygerFrameworkPluginVersion v{};                                                               \
		(versionFn)(v);                                                                                \
		out->Major = v.Major;                                                                          \
		out->Minor = v.Minor;                                                                          \
		out->Patch = v.Patch;                                                                          \
		int mask = 0;                                                                                  \
		for (int g : v.CompatibleGames)                                                                \
			if (g >= 1 && g <= 31) mask |= (1 << (g - 1));                                            \
		out->CompatibleGames = mask;                                                                   \
	}                                                                                                  \
	extern "C" __declspec(dllexport)                                                                   \
	const char* TygerFrameworkPluginInitializeC(const TygerFrameworkPluginInitializeParamC* paramC) { \
		API::BindCInterfaceInternal(paramC);                                                           \
		static TygerFrameworkPluginInitializeParam sParam{};                                           \
		sParam.TyHModule = paramC->TyHModule;                                                          \
		sParam.pluginFileName = paramC->pluginFileName ? paramC->pluginFileName : "";                  \
		sParam.functions = nullptr;                                                                    \
		sParam.initErrorMessage.clear();                                                               \
		bool ok = (initFn)(&sParam);                                                                   \
		if (ok) return nullptr;                                                                        \
		static std::string sErr;                                                                       \
		sErr = sParam.initErrorMessage.empty() ? "Plugin initialization failed" : sParam.initErrorMessage; \
		return sErr.c_str();                                                                           \
	}
