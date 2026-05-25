#pragma once
#include "TygerFrameworkAPI.hpp"

//The C plugin-function table and the init param the loader hands to plugins
//that export the extern "C" entry points.
extern TygerFrameworkPluginFunctionsC pluginFunctionsC;
extern TygerFrameworkPluginInitializeParamC pluginInitParamC;

//Loader helpers for the optional C entry points. Both return false when the
//plugin does not export the C entry point (caller falls back to the C++ path).

//Fills 'out' (including CompatibleGames decoded from the bitmask into the vector).
bool PluginC_TryGetRequiredVersion(void* pluginModule, TygerFrameworkPluginVersion& out);

//Calls TygerFrameworkPluginInitializeC. On found+failure, sets 'outError' and 'outOk=false'.
//On found+success sets 'outOk=true'. Returns false (not handled) if the export is absent.
bool PluginC_TryInitialize(void* pluginModule, void* frameworkModule,
                           const std::string& pluginName, bool& outOk, std::string& outError);
