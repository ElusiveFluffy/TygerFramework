#pragma once
#include <stdint.h>

namespace TyMemoryValues
{
	inline DWORD TyBaseAddress;
	bool HasGameInitialized();
	inline int GetTy1GameState() { return *(int*)(TyBaseAddress + 0x288A6C); };
	void SetVersionText(int tyGame);
	// Only Ty 2 and 3 have a set amount of space available for the text
	void SetTy2And3VersionTextSpace();
	uint32_t* GetPointerAddress(uint32_t* baseAddress, uint32_t offset);
	LPVOID* GetTyShutdownFunc();

	inline char VersionText[50]{};
};