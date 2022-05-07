// Stuff in this file is not used but is just kept here for reference purposes.


// =====================
// UFUnction::NativeBind
// =====================
// Prints addresses of native functions



	// This is only for printing out the natives addresses
	//writeln(L"Initializing UFunction::Bind hook...");
	//if (auto const rc = InterfacePtr->FindPattern(reinterpret_cast<void**>(&UFunctionBind), "48 8B C4 55 41 56 41 57 48 8D A8 78 F8 FF FF 48 81 EC 70 08 00 00 48 C7 44 24 50 FE FF FF FF 48 89 58 10 48 89 70 18 48 89 78 20 48 8B ?? ?? ?? ?? ?? 48 33 C4 48 89 85 60 07 00 00 48 8B F1 E8 ?? ?? ?? ?? 48 8B F8 F7 86");
	//	rc != SPIReturn::Success)
	//{
	//	writeln(L"Attach - failed to find UFunction::Bind pattern: %d / %s", rc, SPIReturnToString(rc));
	//	return false;
	//}
	//if (auto const rc = InterfacePtr->InstallHook(MYHOOK "UFunction::Bind", UFunctionBind, HookedUFunctionBind, reinterpret_cast<void**>(&UFunctionBind_orig));
	//	rc != SPIReturn::Success)
	//{
	//	writeln(L"Attach - failed to hook UFunction::Bind: %d / %s", rc, SPIReturnToString(rc));
	//	return false;
	//}
	//writeln(L"Hooked UFunction::Bind");
	//return true;

// ==========================================
// FConfig::CombineFromBuffer
// ==========================================
/*
 
typedef void (*tCombineFromBuffer)(UObject* Context, wchar_t* filePath, FString* contents, int bRefreshContents);
tCombineFromBuffer FConfigCombineFromBuffer = nullptr;
tCombineFromBuffer FConfigCombineFromBuffer_orig = nullptr;
void FConfigCombineFromBuffer_hook(UObject* Context, wchar_t* filePath, FString* contents, int bRefreshContents)
{
	writeln("FConfig::CombineFromBuffer: %s, bRefreshContents: %i", filePath, bRefreshContents);
	FConfigCombineFromBuffer_orig(Context, filePath, contents, bRefreshContents);
	//FConfigCombineFromBuffer_orig(Context, filePath, contents, 0);
}
 */

//INIT_FIND_PATTERN_POSTHOOK(FConfigCombineFromBuffer,/*40 55 56 57 41*/ "54 41 55 41 56 41 57 48 8d ac 24 b0 fe ff ff 48 81 ec 50 02 00 00 48 c7 85 d0 00 00 00 fe ff ff ff 48 89 9c 24 a8 02 00 00 48 8b 05 c3 c4 5f 01 48 33 c4 48 89 85 40 01 00 00");
//INIT_HOOK_PATTERN(FConfigCombineFromBuffer)