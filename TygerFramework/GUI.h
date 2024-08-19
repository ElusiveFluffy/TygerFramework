#pragma once
namespace GUI
{
	inline bool Initialized;
	inline bool DrawGUI = true;
	static HWND TyWindowHandle;

	bool Init();
	void Draw();
};

