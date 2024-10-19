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

	//Draw Plugin UI
	typedef struct {
		std::string PluginName;
		std::function<void()> Function;
	}DrawPluginUIParam;
	void DrawPluginUI();
	bool AddDrawPluginUIFunc(DrawPluginUIParam param);

	//Plugin ImGui Has Focus
	typedef struct {
		std::string PluginName;
		std::function<bool()> Function;
	}PluginImGuiHasFocusParam;
	bool PluginImGuiWantCaptureMouse();
	bool AddPluginImGuiWantCaptureMouseFunc(PluginImGuiHasFocusParam param);

	//Plugin WndProc
	using TyFPluginWndProc = std::function<bool(HWND, UINT, WPARAM, LPARAM)>;
	typedef struct {
		std::string PluginName;
		TyFPluginWndProc Function;
	}PluginWndProcParam;
	bool PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool AddPluginWndProcFunc(PluginWndProcParam param);

	//Tick Before Game
	typedef struct {
		std::string PluginName;
		std::function<void(float deltaSeconds)> Function;
	}TickBeforeGameParam;
	void TickBeforeGame(float deltaSeconds);
	void AddTickBeforeGameFunc(TickBeforeGameParam param);

	//On Ty Initialized
	typedef struct {
		std::string PluginName;
		std::function<void()> Function;
	}OnTyInitializedParam;
	void OnTyInitialized();
	void AddOnTyInitializedFunc(OnTyInitializedParam param);

	//On Ty Begin Shutdown
	typedef struct {
		std::string PluginName;
		std::function<void()> Function;
	}OnTyBeginShutdownParam;
	void OnTyBeginShutdown();
	void AddOnTyBeginShutdownFunc(OnTyBeginShutdownParam param);
private:
	//Stored Functions
	std::vector<DrawPluginUIParam> mDrawPluginUIParams{};
	std::map<std::string, std::function<bool()>> mPluginImGuiWantCaptureMouseParams{};
	std::vector<PluginWndProcParam> mPluginWndProcParams{};
	std::vector<TickBeforeGameParam> mTickBeforeGameParams{};
	std::vector<OnTyInitializedParam> mOnTyInitializedParams{};
	std::vector<OnTyBeginShutdownParam> mOnTyBeginShutdownParams{};
};