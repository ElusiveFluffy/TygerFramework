#pragma once
namespace GUI
{
	inline bool Initialized;
	inline bool ImGuiWindowFocused;
	inline bool DrawGUI = true;
	static HWND TyWindowHandle;

	bool Init();
	void Draw();
};

