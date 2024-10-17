#include "framework.h"
#include "TyMemoryValues.h"
#include "TygerFramework.h"
#include "GUI.h"

bool TyMemoryValues::HasGameInitialized()
{ 
	switch (FrameworkInstance->CurrentTyGame()) {
	case 1:
		return GetTy1GameState() < 5;
	case 2:
	case 3:
		//Ty 2 and 3 is initialized once the window shows and the GUI only initialized once the window is shown
		return GUI::Initialized;
	default:
		return false;
	}
}

LPVOID* TyMemoryValues::GetTyShutdownFunc()
{
	switch (FrameworkInstance->CurrentTyGame())
	{
	case 1:
		return (LPVOID*)(TyBaseAddress + 0x19DAB0);
	case 2:
		return (LPVOID*)(TyBaseAddress + 0x113400);
	case 3:
		return (LPVOID*)(TyBaseAddress + 0x2EBE10);
	default:
		return nullptr;
	}
}
