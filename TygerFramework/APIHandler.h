#pragma once
#include <vector>
#include <functional>
#include <memory>

class APIHandler
{
public:
	static std::shared_ptr<APIHandler>& Get();

	void DrawPluginUI();
	bool AddDrawPluginUIFunc(std::function<void()> func);

	bool PluginImGuiHasFocus();
	bool AddPluginImGuiHasFocusFunc(std::function<bool()> func);

	using TyFPluginWndProc = std::function<bool(HWND, UINT, WPARAM, LPARAM)>;
	bool PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool AddPluginWndProcFunc(TyFPluginWndProc func);

	void TickBeforeGame(float deltaSeconds);
	void AddTickBeforeGameFunc(std::function<void(float deltaSeconds)> func);
private:
	std::vector<std::function<void()>> mDrawPluginUIFunctions{};
	std::vector<std::function<bool()>> mPluginImGuiHasFocusFunctions{};
	std::vector<TyFPluginWndProc> mPluginWndProcFunctions{};
	std::vector<std::function<void(float deltaSeconds)>> mTickBeforeGameFunctions{};
};