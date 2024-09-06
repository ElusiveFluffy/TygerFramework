#include "API.h"
#include <memory>
#include <stdexcept>

namespace TygerFrameworkAPI {
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

		/// <summary>
		/// Gets the current Ty Game
		/// </summary>
		/// <returns>0: Couldn't detect which game
		/// <para>1: Ty 1</para>
		/// <para>2: Ty 2</para>
		/// <para>3: Ty 3</para></returns>
		static int WhichTyGame() {
			return Get()->param()->functions->WhichTyGame();
		}

		//Writes a message to the console and the log file. Default log level is info
		static void LogPluginMessage(std::string message, LogLevel logLevel = Info) {
			Get()->param()->functions->LogPluginMessage("[" + PluginName + "] " + message, logLevel);
		}

		//Returns the current ImGui Context 
		//(Make sure to cast it to ImGuiContext* when setting it. Its a void* so it doesn't cause a error if you don't have ImGui)
		static void* GetImGuiContext() {
			return API::Get()->param()->functions->GetImGuiContext();
		}

	private:
		static inline std::unique_ptr<API> mInstance;
		const TygerFrameworkPluginInitializeParam* mParam;
	};
}