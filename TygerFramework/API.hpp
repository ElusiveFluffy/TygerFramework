#include "API.h"
#include <memory>
#include <stdexcept>

namespace TygerFrameworkAPI {
	class API {
	public:
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

		API(const TygerFrameworkPluginInitializeParam* param)
			: mParam{ param }
		{
		}

		inline const auto param() const {
			return mParam;
		}

	private:
		static inline std::unique_ptr<API> mInstance;
		const TygerFrameworkPluginInitializeParam* mParam;
	};
}