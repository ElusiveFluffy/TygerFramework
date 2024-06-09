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

		void LogPluginMessage(std::string message, LogLevel logLevel = Info) const {
			param()->functions->LogPluginMessage("[" + PluginName + "] " + message, logLevel);
		}

	private:
		static inline std::unique_ptr<API> mInstance;
		const TygerFrameworkPluginInitializeParam* mParam;
	};
}