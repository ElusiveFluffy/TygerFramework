#pragma once
#include <vector>
#include <functional>
#include <string>
#include <memory>

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
	bool PluginImGuiHasFocus();
	bool AddPluginImGuiHasFocusFunc(PluginImGuiHasFocusParam param);

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
private:
	//Stored Functions
	std::vector<DrawPluginUIParam> mDrawPluginUIParams{};
	std::vector<PluginImGuiHasFocusParam> mPluginImGuiHasFocusParams{};
	std::vector<PluginWndProcParam> mPluginWndProcParams{};
	std::vector<TickBeforeGameParam> mTickBeforeGameParams{};
};