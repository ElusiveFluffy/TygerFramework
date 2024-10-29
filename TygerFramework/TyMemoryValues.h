#pragma once
namespace TyMemoryValues
{
	inline DWORD TyBaseAddress;
	bool HasGameInitialized();
	inline int GetTy1GameState() { return *(int*)(TyBaseAddress + 0x288A6C); };
	void SetTy1VersionText();
	LPVOID* GetTyShutdownFunc();

	inline char VersionText[40];
};