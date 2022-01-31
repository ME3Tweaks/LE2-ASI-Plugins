#include <stdio.h>
#include <io.h>
#include <string>
#include <fstream>
#include "../LE2-SDK/Interface.h"
#include "../LE2-SDK/Common.h"
#include "../LE2-SDK/ME3TweaksHeader.h"
#define MYHOOK "DebugLogger_"

SPI_PLUGINSIDE_SUPPORT(L"DebugLogger", L"2.0.0", L"ME3Tweaks", SPI_GAME_LE2, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_PRELOAD;
SPI_PLUGINSIDE_SEQATTACH;

ME3TweaksASILogger logger("DebugLogger v1", "DebugLogger.txt");

// Original Func
typedef void (WINAPI* tOutputDebugStringW)(LPCWSTR lpcszString);
tOutputDebugStringW OutputDebugStringW_Orig = nullptr;

// Our replacement
void WINAPI OutputDebugStringW_Hook(LPCWSTR lpcszString)
{
	OutputDebugStringW_Orig(lpcszString);
	writeMsg(L"%s", lpcszString); // string already has a newline on the end
	logger.writeWideToLog(std::wstring_view{ lpcszString });
	logger.flush();
}

typedef UObject* (*tCreateImport)(ULinkerLoad* Context, int UIndex);
tCreateImport CreateImport = nullptr;
tCreateImport CreateImport_orig = nullptr;
UObject* CreateImport_hook(ULinkerLoad* Context, int i)
{
	UObject* object = CreateImport_orig(Context, i);
	if (object == nullptr)
	{
		FObjectImport importEntry = Context->ImportMap(i);
		//writeln("Could not resolve #%d: %hs (%hs) in file: %s", -i - 1, importEntry.ObjectName.GetName(), importEntry.ClassName.GetName(), Context->Filename.Data);
		logger.writeWideLineToLog(wstring_format(L"Could not resolve #%d: %hs (%hs) in file: %s", -i - 1, importEntry.ObjectName.GetName(), importEntry.ClassName.GetName(), Context->Filename.Data));
		logger.flush();
	}
	return object;
}

///===============================
/// METHOD PROTOTYPES
///===============================
// LOGF
typedef void (*tFOutputDeviceLogF)(void* outputDevice, wchar_t* formatStr, void* param1, void* param2);
tFOutputDeviceLogF LogF = nullptr;
tFOutputDeviceLogF LogF_orig = nullptr;
void LogF_hook(void* fOutputDevice, wchar_t* formatStr, void* param1, void* param2)
{
	fwprintf_s(stdout, L"appLogf: ");
	fwprintf_s(stdout, formatStr, param1, param2);
	fwprintf_s(stdout, L"\n");
}

//typedef void (*tFOutputDeviceLogF)(void* param1, wchar_t* param2, wchar_t* param3);
tFOutputDeviceLogF MsgF = nullptr;
tFOutputDeviceLogF MsgF_orig = nullptr;
void MsgF_hook(void* fOutputDevice, wchar_t* formatStr, wchar_t* param1)
{
	fwprintf_s(stdout, L"appMsgf: ");
	fwprintf_s(stdout, formatStr, param1);
	fwprintf_s(stdout, L"\n");
}

typedef void (*tMessageBoxF)(int messageBoxType, wchar_t* formatString, wchar_t* param3);
tMessageBoxF MsgFDialog = nullptr;
tMessageBoxF MsgFDialog_orig = nullptr;
void MsgFDialog_hook(int messageBoxType, wchar_t* formatStr, wchar_t* param1)
{
	MessageBoxW(NULL, formatStr, L"Game message", 0x0);
}

typedef void (*tCombineFromBuffer)(UObject* Context, wchar_t* filePath, FString* contents, int bRefreshContents);
tCombineFromBuffer FConfigCombineFromBuffer = nullptr;
tCombineFromBuffer FConfigCombineFromBuffer_orig = nullptr;
void FConfigCombineFromBuffer_hook(UObject* Context, wchar_t* filePath, FString* contents, int bRefreshContents)
{
	writeln("FConfig::CombineFromBuffer: %s, bRefreshContents: %i", filePath, bRefreshContents);
	FConfigCombineFromBuffer_orig(Context, filePath, contents, bRefreshContents);
	//FConfigCombineFromBuffer_orig(Context, filePath, contents, 0);
}


// ======================================
// LOGGING HOOKS
// ======================================
void hookLoggingFunc(ISharedProxyInterface* InterfacePtr)
{
	// ============================================================
	// appLogF
	//=============================================================
	writeln(L"Initializing appLogf hook...");
	if (auto const rc = InterfacePtr->FindPattern(reinterpret_cast<void**>(&LogF), "48 8b c4 48 89 50 10 4c 89 40 18 4c 89 48 20 56 48 83 ec 50 83 79 08 00 48 8b f1 0f 85 bf 00 00 00");
		rc != SPIReturn::Success)
	{
		writeln(L"Attach - failed to find appLogf pattern: %d / %s", rc, SPIReturnToString(rc));
		//return false;
	}
	else if (auto const rc = InterfacePtr->InstallHook(MYHOOK "appLogf", LogF, LogF_hook, reinterpret_cast<void**>(&LogF_orig));
		rc != SPIReturn::Success)
	{
		writeln(L"Attach - failed to hook appLogf: %d / %s", rc, SPIReturnToString(rc));
		//return false;
	}
	else
	{
		writeln(L"Hooked appLogf");
	}

	// ==============================================================
// FConfig::CombineFromBuffer
// ==============================================================
	writeln(L"Locating FConfig::CombineFromBuffer...");
	if (auto const rc = InterfacePtr->FindPattern(reinterpret_cast<void**>(&FConfigCombineFromBuffer), "40 55 56 57 41 54 41 55 41 56 41 57 48 8d ac 24 b0 fe ff ff 48 81 ec 50 02 00 00 48 c7 85 d0 00 00 00 fe ff ff ff 48 89 9c 24 a8 02 00 00 48 8b 05 c3 c4 5f 01 48 33 c4 48 89 85 40 01 00 00");
		rc != SPIReturn::Success)
	{
		writeln(L"Failed to find FConfig::CombineFromBuffer pattern: %d / %s", rc, SPIReturnToString(rc));
		//return false;
	}
	else if (auto const rc = InterfacePtr->InstallHook(MYHOOK "FConfigCombineFromBuffer", FConfigCombineFromBuffer, FConfigCombineFromBuffer_hook, reinterpret_cast<void**>(&FConfigCombineFromBuffer_orig));
		rc != SPIReturn::Success)
	{
		writeln(L"Attach - failed to hook FConfigCombineFromBuffer: %d / %s", rc, SPIReturnToString(rc));
		//return false;
	}
	else
	{
		writeln(L"Hooked FConfigCombineFromBuffer");
	}
}

SPI_IMPLEMENT_ATTACH
{
	Common::OpenConsole();
	writeln(L"Initializing DebugLogger...");

	if (auto rc = InterfacePtr->InstallHook("OutputDebugStringW", (void*)OutputDebugStringW, (void*)OutputDebugStringW_Hook, (void**)&OutputDebugStringW_Orig);
		rc != SPIReturn::Success)
	{
		writeln(L"Attach - failed to hook OutputDebugStringW: %d / %s", rc, SPIReturnToString(rc));
		return false;
	}
	writeln(L"Initialized DebugLogger");


	auto _ = SDKInitializer::Instance();
	writeln(L"Initializing CreateImport hook...");
	if (auto const rc = InterfacePtr->FindPattern(reinterpret_cast<void**>(&CreateImport), "48 8b c4 55 41 54 41 55 41 56 41 57 48 8b ec 48 83 ec 70 48 c7 45 d0 fe ff ff ff 48 89 58 10 48 89 70 18 48 89 78 20 4c 63 e2");
		rc != SPIReturn::Success)
	{
		writeln(L"Attach - failed to find CreateImport pattern: %d / %s", rc, SPIReturnToString(rc));
		return false;
	}
	if (auto const rc = InterfacePtr->InstallHook(MYHOOK "CreateImport", CreateImport, CreateImport_hook, reinterpret_cast<void**>(&CreateImport_orig));
		rc != SPIReturn::Success)
	{
		writeln(L"Attach - failed to hook CreateImport: %d / %s", rc, SPIReturnToString(rc));
		return false;
	}
	writeln(L"Hooked CreateImport");

	hookLoggingFunc(InterfacePtr);

	return true;
}


SPI_IMPLEMENT_DETACH
{
	//DebugActiveProcessStop(GetCurrentProcessId());
	Common::CloseConsole();
	logger.close();
	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID) {
	if (reason == DLL_PROCESS_DETACH)
	{
		logger.close();
	}

	return TRUE;
}