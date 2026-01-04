#pragma once
namespace GUI
{
	inline bool Initialized = false;
	inline bool ImGuiWindowFocused = false;
	inline bool DrawGUI = true;
	static float FontSize = 16;

	bool Init();
	void Draw();

	void SetImGuiStyle();
};

