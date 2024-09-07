#pragma once
namespace GUI
{
	inline bool Initialized;
	inline bool ImGuiWindowFocused;
	inline bool DrawGUI = true;
	static float FontSize = 16;

	bool Init();
	void Draw();

	void SetImGuiStyle();
};

