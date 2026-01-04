#include "framework.h"
#include "TyMemoryValues.h"
#include "TygerFramework.h"
#include "GUI.h"
#include <format>
#include <array>

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

void TyMemoryValues::SetVersionText(int tyGame)
{
	//Pointer to the original text
	char** textPointer = nullptr;
	switch (tyGame)
	{
	case 1:
		textPointer = (char**)(TyBaseAddress + 0xe1486);
		break;
	case 2:
		textPointer = (char**)(TyBaseAddress + 0x4bc07c);
		break;
	case 3:
		textPointer = (char**)(TyBaseAddress + 0x48ccb0);
	}
	if (!textPointer)
		return;
	//Get the original text
	char* originalVersionText = *textPointer;

	//Edit the version text, adding the original text onto the end
	strcpy_s(VersionText, std::format("TygerFramework v{}.{}.{} | ", TygerFrameworkPluginVersion_Major, TygerFrameworkPluginVersion_Minor, TygerFrameworkPluginVersion_Patch).c_str());
	strcat_s(VersionText, originalVersionText);

	DWORD oldProtection;
	//Change the memory access to ReadWrite to be able to change the hardcoded value (usually its read only)
	VirtualProtect(textPointer, 4, PAGE_EXECUTE_READWRITE, &oldProtection);

	//Set the pointer to TygerFramework's text
	*textPointer = VersionText;

	//Set it back to the old access protection
	VirtualProtect(textPointer, 4, oldProtection, &oldProtection);
}

void TyMemoryValues::SetTy2And3VersionTextSpace()
{
	uint32_t* versionTextAvaliableWidth = nullptr;
	std::array<uint32_t, 7> offsets;
	switch (FrameworkInstance->CurrentTyGame())
	{
	case 2:
		versionTextAvaliableWidth = (uint32_t*)(TyBaseAddress + 0x4EDD4C);
		offsets = { 0x4FC, 0x564, 0x10, 0x10, 0x10, 0x10, 0x4C };
		break;
	case 3:
		versionTextAvaliableWidth = (uint32_t*)(TyBaseAddress + 0x4C653C);
		offsets = { 0x10, 0x64, 0x64, 0x10, 0x568, 0x10, 0x54 };
		break;
	default:
		return;
	}

	for (auto& offset : offsets)
	{
		versionTextAvaliableWidth = GetPointerAddress(versionTextAvaliableWidth, offset);
		// The first pointer will be null for Ty 2 before the menu loads
		// The first pointer will be null for Ty 2 and 3 during gameplay
		if (!versionTextAvaliableWidth)
			return;
	}

	*(float*)versionTextAvaliableWidth = 800.0f;
}

uint32_t* TyMemoryValues::GetPointerAddress(uint32_t* baseAddress, uint32_t offset)
{
	if (*baseAddress == 0)
		return nullptr;
	//Use the address to get a pointer then add the offset to that pointer
	return (uint32_t*)(*baseAddress + offset);
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
