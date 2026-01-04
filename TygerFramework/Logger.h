#pragma once
#include "TygerFrameworkAPI.hpp"
#include <string>
#include <fstream>

inline std::ofstream mLogger;

namespace Logger {
	void StartLogger();
	std::string GetTimeStamp();
	void LogMessage(std::string message, LogLevel errorType = Info);
}

