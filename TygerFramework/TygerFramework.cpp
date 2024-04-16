#include "pch.h"
#include "TygerFramework.h"
#include <filesystem>
namespace fs = std::filesystem;

std::filesystem::path TygerFramework::GetPluginDir() {
    return fs::current_path() / "Plugins";
}
