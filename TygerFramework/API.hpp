#include "API.h"
#include <memory>
#include <stdexcept>

namespace TygerFrameworkAPI {
	class API {
	public:
		//Optional, name to be used in the logs
		static inline std::string PluginName = "Plugin";

		static auto& Initialize(const TygerFrameworkPluginInitializeParam* param) {
			if (param == nullptr) {
				throw std::runtime_error("param is null");
			}

			if (mInstance != nullptr) {
				throw std::runtime_error("API already initialized");
			}

			mInstance = std::make_unique<API>(param);
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