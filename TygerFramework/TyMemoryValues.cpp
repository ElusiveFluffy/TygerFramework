#include "framework.h"
#include "TyMemoryValues.h"
#include "TygerFramework.h"
#include "GUI.h"
#include <format>

char TyMemoryValues::VersionText[36];

bool TyMemoryValues::HasGameInitialized()
{ 
	switch (FrameworkInstance->CurrentTyGame()) {
	case 1:
		return GetTy1GameState() < 5;
	case 2:
	case 3:
		//Ty 2 and 3 is initialized once the window shows and the GUI only initialized once the window is shown
		return !GUI::Initialized;
	default:
		return false;
	}
}

void TyMemoryValues::SetTy1VersionText()
{
	//Pointer to the original text
	int* textPointer = (int*)(TyBaseAddress + 0xe1486);
	//Get the original text
	char* originalVersionText = (char*)(*textPointer);

	//Edit the version text, adding the original text onto the end
	strcpy_s(VersionText, std::format("TygerFramework v{}.{}.{} | ", TygerFrameworkPluginVersion_Major, TygerFrameworkPluginVersion_Minor, TygerFrameworkPluginVersion_Patch).c_str());
	strcat_s(VersionText, originalVersionText);

	DWORD oldProtection;
	//Change the memory access to ReadWrite to be able to change the hardcoded value (usually its read only)
	VirtualProtect(textPointer, 4, PAGE_EXECUTE_READWRITE, &oldProtection);

	//Set the pointer to TygerFramework's text
	*textPointer = (int)&VersionText;

	//Set it back to the old access protection
	VirtualProtect(textPointer, 4, oldProtection, &oldProtection);
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
