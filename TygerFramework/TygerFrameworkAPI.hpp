#include "TygerFrameworkAPI.h"
#include <memory>
#include <stdexcept>

class API {
public:
	//Will default to the plugin's file name on initialization, the name will be used in the logs
	//If you want a different name, can change the plugin name in DllMain
	static inline std::string PluginName = "Plugin";

	static auto& Initialize(const TygerFrameworkPluginInitializeParam* param) {
		if (param == nullptr) {
			throw std::runtime_error("param is null");
		}

		if (mInstance != nullptr) {
			throw std::runtime_error("API already initialized");
		}

		mInstance = std::make_unique<API>(param);

		//If the plugin name has already been changed to something custom it won't change it to the file name
		if (PluginName == "Plugin")
			PluginName = param->pluginFileName;

		return mInstance;
	}

	//Only call this AFTER calling initialize
	static auto& Get() {
		if (mInstance == nullptr) {
			throw std::runtime_error("API not initialized");
		}

		return mInstance;
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

	//Gets if TygerFramework is drawing the GUI. Only use for imgui if you have a special use case
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

	//Writes a message to the console and the log file. Default log level is info
	static void LogPluginMessage(std::string message, LogLevel logLevel = Info) {
		Get()->param()->functions->LogPluginMessage("[" + PluginName + "] " + message, logLevel);
	}

	//Sets the imgui font to be the same as TygerFramework's main window
	static void SetImGuiFont(void* imguiFont) {
		Get()->param()->functions->SetImGuiFont(imguiFont);
	}

	//Sets all the elements for drawing elements from the plugin in the TygerFramework window
	//Overwrites the old value if its called again
	static void SetTygerFrameworkImGuiElements(std::vector<TygerFrameworkImGuiParam> elements) {
		Get()->param()->functions->SetTyFImGuiElements(PluginName, elements);
	}

	//--------------------------
	//Event subscriber functions
	//--------------------------

	static bool AddDrawPluginUI(DrawUIFunc func) {
		return Get()->param()->functions->AddDrawPluginUI(PluginName, func);
	}

	static bool AddPluginImGuiHasFocus(ImGuiHasFocusFunc func) {
		return Get()->param()->functions->AddPluginImGuiHasFocus(PluginName, func);
	}

	static bool AddPluginWndProc(WndProcFunc func) {
		return Get()->param()->functions->AddPluginWndProc(PluginName, func);
	}

	static bool AddTickBeforeGame(TickBeforeGameFunc func) {
		return Get()->param()->functions->AddTickBeforeGame(PluginName, func);
	}

private:
	static inline std::unique_ptr<API> mInstance;
	const TygerFrameworkPluginInitializeParam* mParam;
};