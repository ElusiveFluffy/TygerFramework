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

	using TyFPluginWndProc = std::function<bool (HWND, UINT, WPARAM, LPARAM)>;
	bool PluginWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool AddPluginWndProcFunc(TyFPluginWndProc func);
private:
	std::vector<std::function<void()>> mDrawPluginUIFunctions{};
	std::vector<TyFPluginWndProc> mPluginWndProcFunctions{};
};

