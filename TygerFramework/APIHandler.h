#pragma once
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include <map>

class APIHandler
{
public:
	static std::shared_ptr<APIHandler>& Get();

	typedef struct {
		std::string PluginName;
		std::function<void()> Function;
	}VoidFunctionParam;

	typedef struct {
		std::string PluginName;
		std::function<bool()> Function;
	}PluginWantCaptureMouseParam;

	using TyFPluginWndProc = std::function<bool(HWND, UINT, WPARAM, LPARAM)>;
	typedef struct {
		std::string PluginName;
		TyFPluginWndProc Function;
	}PluginWndProcParam;

	typedef struct {
		std::string PluginName;
		std::function<void(float deltaSeconds)> Function;
	}TickBeforeGameParam;

	//Draw Plugin UI
	void DrawPluginUI();
	bool AddDrawPluginUIFunc(VoidFunctionParam param);

	//Plugin ImGui Has Focus
	bool PluginImGuiWantCaptureMouse();
	bool AddPluginImGuiWantCaptureMouseFunc(PluginWantCaptureMouseParam param);

	//Plugin WndProc
	bool PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool AddPluginWndProcFunc(PluginWndProcParam param);

	//Tick Before Game
	void TickBeforeGame(float deltaSeconds);
	void AddTickBeforeGameFunc(TickBeforeGameParam param);

	//On Ty Initialized
	void OnTyInitialized();
	void AddOnTyInitializedFunc(VoidFunctionParam param);

	//On Ty Begin Shutdown
	void OnTyBeginShutdown();
	void AddOnTyBeginShutdownFunc(VoidFunctionParam param);
private:
	//Stored Functions
	std::vector<VoidFunctionParam> mDrawPluginUIParams{};
	std::map<std::string, std::function<bool()>> mPluginImGuiWantCaptureMouseParams{};
	std::vector<PluginWndProcParam> mPluginWndProcParams{};
	std::vector<TickBeforeGameParam> mTickBeforeGameParams{};
	std::vector<VoidFunctionParam> mOnTyInitializedParams{};
	std::vector<VoidFunctionParam> mOnTyBeginShutdownParams{};
};