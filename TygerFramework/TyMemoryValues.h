#pragma once
class TyMemoryValues
{
public:
	static inline DWORD TyBaseAddress;
	static bool HasGameInitialized();
	static int GetTy1GameState() { return *(int*)(TyBaseAddress + 0x288A6C); };
	static LPVOID* GetTyShutdownFunc();
};