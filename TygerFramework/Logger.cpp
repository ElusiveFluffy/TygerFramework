#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>

void Logger::StartLogger()
{
    mLogger.open("TygerFrameworkLog.txt");
}

std::string Logger::GetTimeStamp()
{
    using namespace std::chrono;

    auto current_time_point = system_clock::now();
    time_t current_time = system_clock::to_time_t(current_time_point);
    tm current_localtime;
    localtime_s(&current_localtime, &current_time);
    auto current_time_since_epoch = current_time_point.time_since_epoch();
    auto current_milliseconds = duration_cast<milliseconds> (current_time_since_epoch).count() % 1000;

    std::ostringstream stream;
    stream << std::put_time(&current_localtime, "%T") << "." << std::setw(3) << std::setfill('0') << current_milliseconds;
    return "[" + stream.str() + "] ";
}

void Logger::LogMessage(std::string message, LogLevel logLevel) {
    std::string logLevelString;

    if (mLogger.is_open())
    {
        switch (logLevel)
        {
        case Info:
            logLevelString = "[Info] ";
            break;
        case Warning:
            logLevelString = "[Warning] ";
            break;
        case Error:
            logLevelString = "[Error] ";
            break;
        default:
            break;
        }
        std::string timeStamp = GetTimeStamp();
        mLogger << timeStamp << logLevelString << message << std::endl;
        std::cout << timeStamp << logLevelString << message << std::endl;
    }
    else {
        std::ofstream outfile("LoggerErrors.txt");

        outfile << "Error Logger Isn't Running!" << std::endl;

        outfile.close();
    }
}