#include "framework.h"
#include "PluginFunctionsC.h"
#include "PluginLoader.h"
#include "TygerFramework.h"
#include "Logger.h"
#include <vector>
#include "Minhook.h"

//---- C thunks: convert C types to STL and forward to the existing internals ----

static void LogPluginMessageC(const char* message, int logLevel) {
	Logger::LogMessage(message ? message : "", (LogLevel)logLevel);
}

static int CurrentTyGameC() {
	return FrameworkInstance->CurrentTyGame();
}

static bool AddDrawPluginUIC(const char* pluginName, VoidFunc func) {
	return TygerFrameworkDrawPluginUi(pluginName ? pluginName : "", func);
}

static bool AddPluginImGuiWantCaptureMouseC(const char* pluginName, ImGuiWantCaptureMouseFunc func) {
	return TygerFrameworkPluginImguiWantCaptureMouse(pluginName ? pluginName : "", func);
}

static bool AddPluginWndProcC(const char* pluginName, WndProcFunc func) {
	return TygerFrameworkPluginWndProc(pluginName ? pluginName : "", func);
}

static void SetTyFImGuiElementsC(const char* pluginName, const TygerFrameworkImGuiParamC* params, int count) {
	std::vector<TygerFrameworkImGuiParam> elements;
	elements.reserve(count > 0 ? count : 0);
	for (int i = 0; i < count; ++i)
		elements.push_back(TygerFrameworkImGuiParam{ (TyFImGuiElements)params[i].ImGuiElement,
		                                              params[i].Text ? params[i].Text : "" });
	PluginSetTygerFrameworkImGuiElements(pluginName ? pluginName : "", elements);
}

static bool AddTickBeforeGameC(const char* pluginName, TickBeforeGameFunc func) {
	return TygerFrameworkTickBeforeGame(pluginName ? pluginName : "", func);
}

static bool AddOnTyInitializedC(const char* pluginName, VoidFunc func) {
	return TygerFrameworkOnTyInitialized(pluginName ? pluginName : "", func);
}

static bool AddOnTyBeginShutdownC(const char* pluginName, VoidFunc func) {
	return TygerFrameworkOnTyBeginShutdown(pluginName ? pluginName : "", func);
}

static bool SetTyInputStateC(const char* pluginName, int flags) {
	return PluginSetTyInputStateProxy(pluginName ? pluginName : "", (TyInputsFlags)flags);
}

static int GetTyInputStateC(const char* pluginName) {
	return (int)GetPluginsTyInputState(pluginName ? pluginName : "");
}

static const char* GetPluginDirC() {
	//Framework-owned, computed once, stable for the process lifetime.
	static std::string cached = TygerFramework::GetPluginDir().string();
	return cached.c_str();
}

static bool CreateHook(void* pTarget, void* pDetour, void** ppOriginal, const char* pPluginName) {
	MH_STATUS minHookStatus = MH_CreateHook(pTarget, pDetour, ppOriginal);
	if (minHookStatus != MH_OK) {
		std::string strPluginName = pPluginName ? " for " + std::string(pPluginName) : "";
		std::string error = MH_StatusToString(minHookStatus);
		Logger::LogMessage("[API] Failed to Create Function Hook at " + std::to_string((int)pTarget) + strPluginName + ", With the Error : " + error, Error);
		return false;
	}
	minHookStatus = MH_EnableHook(pTarget);
	if (minHookStatus != MH_OK) {
		std::string strPluginName = pPluginName ? " for " + std::string(pPluginName) : "";
		std::string error = MH_StatusToString(minHookStatus);
		Logger::LogMessage("[API] Failed to Enable Function Hook at " + std::to_string((int)pTarget) + strPluginName + ", With the Error: " + error, Error);
		return false;
	}
	return true;
}

static bool DestroyHook(void* pTarget, const char* pPluginName) {
	MH_STATUS minHookStatus = MH_DisableHook(pTarget);
	if (minHookStatus != MH_OK) {
		std::string strPluginName = pPluginName ? " for " + std::string(pPluginName) : "";
		std::string error = MH_StatusToString(minHookStatus);
		Logger::LogMessage("[API] Failed Disable Function Hook at " + std::to_string((int)pTarget) + strPluginName + ", With the Error : " + error, Error);
		return false;
	}
	minHookStatus = MH_RemoveHook(pTarget);
	if (minHookStatus != MH_OK) {
		std::string strPluginName = pPluginName ? " for " + std::string(pPluginName) : "";
		std::string error = MH_StatusToString(minHookStatus);
		Logger::LogMessage("[API] Failed to Remove Function Hook at " + std::to_string((int)pTarget) + strPluginName + ", With the Error: " + error, Error);
		return false;
	}
	return true;
}

//---- the table + init param ----

TygerFrameworkPluginFunctionsC pluginFunctionsC{
	LogPluginMessageC,
	CurrentTyGameC,
	AddDrawPluginUIC,
	AddPluginImGuiWantCaptureMouseC,
	AddPluginWndProcC,
	TygerFrameworkGetTyWindowHandle,   //HWND() -- already C-compatible
	TygerFrameworkDrawingGUI,          //bool() -- already C-compatible
	//NB: the C++ struct's SetImGuiFont slot is intentionally absent here --
	//it takes a raw ImFontAtlas* which is not C-ABI stable, so the C surface drops it.
	SetTyFImGuiElementsC,
	AddTickBeforeGameC,
	AddOnTyInitializedC,
	AddOnTyBeginShutdownC,
	SetTyInputStateC,
	GetTyInputStateC,
	GetPluginDirC,
	CreateHook,
	DestroyHook,
};

TygerFrameworkPluginInitializeParamC pluginInitParamC{
	nullptr,
	"",
	&pluginFunctionsC,
};

//---- loader helpers for the optional C entry points ----

bool PluginC_TryGetRequiredVersion(void* pluginModule, TygerFrameworkPluginVersion& out) {
	auto fn = (void(*)(TygerFrameworkPluginVersionC*))GetProcAddress((HMODULE)pluginModule, "TygerFrameworkPluginRequiredVersionC");
	if (fn == nullptr)
		return false;

	TygerFrameworkPluginVersionC vc{};
	fn(&vc);
	out.Major = vc.Major;
	out.Minor = vc.Minor;
	out.Patch = vc.Patch;
	out.CompatibleGames.clear();
	for (int g = 1; g <= 31; ++g)
		if (vc.CompatibleGames & (1 << (g - 1)))
			out.CompatibleGames.push_back(g);
	return true;
}

bool PluginC_TryInitialize(void* pluginModule, void* frameworkModule,
                           const std::string& pluginName, bool& outOk, std::string& outError) {
	auto fn = (const char*(*)(const TygerFrameworkPluginInitializeParamC*))GetProcAddress((HMODULE)pluginModule, "TygerFrameworkPluginInitializeC");
	if (fn == nullptr)
		return false;

	pluginInitParamC.TyHModule = frameworkModule;
	pluginInitParamC.pluginFileName = pluginName.c_str();
	const char* err = fn(&pluginInitParamC);
	outOk = (err == nullptr);
	outError = (err != nullptr) ? err : "";
	return true;
}

// dllexports for dependency dlls, since they don't use the API
// Functions for dependencies to be able to name the plugin that was trying to use the hook, or the dependency's name, to help with tracking hook errors
extern "C" __declspec(dllexport) bool Framework_CreateHook(void* pTarget, void* pDetour, void** ppOriginal, const char* pSourceName) {
	return CreateHook(pTarget, pDetour, ppOriginal, pSourceName);
}

extern "C" __declspec(dllexport) bool Framework_DestroyHook(void* pTarget, const char* pSourceName) {
	return DestroyHook(pTarget, pSourceName);
}