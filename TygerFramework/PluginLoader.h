#pragma once
#include <map>
#include <string>
class PluginLoader
{
public:
	void EarlyInit();

private:
	std::map<std::string, HMODULE> mPlugins{};
};

