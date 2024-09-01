#pragma once
#include <map>
#include <string>
#include "API.hpp"

class PluginLoader
{
public:
	void EarlyInit();
	void Initialize();
	void DrawUI();

private:
	std::map<std::string, HMODULE> mPlugins{};
};

