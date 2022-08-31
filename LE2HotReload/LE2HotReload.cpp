#include "../../Shared-ASI/Interface.h"
#include "../../Shared-ASI/Common.h"

SPI_PLUGINSIDE_SUPPORT(L"LE2HotReload", L"1.0.0", L"ME3Tweaks", SPI_GAME_LE2, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_PRELOAD;
SPI_PLUGINSIDE_ASYNCATTACH;

/**
 * \brief Patches a segment of process memory.
 * \param address The address to start the overwrite of patch data to
 * \param patch The patch data
 * \param patchSize The size of patch
 * \return true if patched, false otherwise
 */
bool PatchMemory(void* address, const void* patch, const SIZE_T patchSize)
{
	//make the memory we're going to patch writeable
	DWORD  oldProtect;
	if (!VirtualProtect(address, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect))
		return false;

	//overwrite with our patch
	memcpy(address, patch, patchSize);

	//restore the memory's old protection level
	VirtualProtect(address, patchSize, oldProtect, &oldProtect);
	FlushInstructionCache(GetCurrentProcess(), address, patchSize);
	return true;
}

void* PatchOffset;
const BYTE patchData[] = { 0x03 }; // Original in 0x1 : READ SHARE (NOT WRITE). 3 is 0x1 | 0x2 which is READ WRITE SHARE
SPI_IMPLEMENT_ATTACH
{
	//Common::OpenConsole();
	INIT_FIND_PATTERN(PatchOffset, "01 ff 15 97 41 f0 00 48 89 06 48 83 f8");
	if (PatchOffset != nullptr)
	{
		PatchMemory(PatchOffset, patchData , 1);
		writeln("Patched for hot reload at %p", PatchOffset);
	} else
	{
		writeln("Failed to patch for hot reload");
	}
}


SPI_IMPLEMENT_DETACH
{
	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID) {
	return TRUE;
}