#pragma once
#include <map>
#include <string>
#include "API.hpp"

class PluginLoader
{
public:
	void EarlyInit();
	void Initialize();

private:
	std::map<std::string, HMODULE> mPlugins{};
};

