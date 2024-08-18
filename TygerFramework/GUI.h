#pragma once
namespace GUI
{
	inline bool Initialized;
	inline bool DrawUI = true;
	static HWND TyWindowHandle;

	bool Init();
	void Draw();
};

