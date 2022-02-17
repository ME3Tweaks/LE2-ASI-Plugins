#include <chrono>
#include <stdio.h>
#include <io.h>
#include <string>
#include <fstream>
#include <set>
#include <stack>
#include <thread>

#include "../LE2-SDK/Interface.h"
#include "../LE2-SDK/Common.h"
#include "../LE2-SDK/ME3TweaksHeader.h"
#include "HookPrototypes.h"

#define MYHOOK "DebugLogger_"

SPI_PLUGINSIDE_SUPPORT(L"DebugLogger", L"2.0.0", L"ME3Tweaks", SPI_GAME_LE2, SPI_VERSION_LATEST);
SPI_PLUGINSIDE_PRELOAD;
SPI_PLUGINSIDE_SEQATTACH;

ME3TweaksASILogger logger("DebugLogger v2", "DebugLogger.txt");

// ===========================
// Debug output from game
// ===========================
typedef void (WINAPI* tOutputDebugStringW)(LPCWSTR lpcszString);
tOutputDebugStringW OutputDebugStringW_orig = nullptr;
void WINAPI OutputDebugStringW_hook(LPCWSTR lpcszString)
{
	OutputDebugStringW_orig(lpcszString);
	writeMsg(L"%s", lpcszString); // string already has a newline on the end
	logger.writeWideToLog(std::wstring_view{ lpcszString });
	logger.flush();
}

// ==========================
// CreateImport
// ==========================
char* GetFullImportName(ULinkerLoad* Context, FObjectImport import)
{
	stack<FObjectResource> parentStack;
	static char cOutBuffer[256];
	int outerIdx = import.OuterIndex;
	//auto i1 = Context->ExportMap(0);
	//auto i2 = Context->ExportMap(1);
	//auto i3 = Context->ExportMap(2);

	parentStack.push(import); // push self so we are on the end
	while (outerIdx != 0) {
		if (outerIdx > 0) {
			// This import is nested under an export
			auto exp = Context->ExportMap(outerIdx - 1);
			exp.Object->Linker;
			parentStack.push(exp);
			outerIdx = exp.OuterIndex;
		}
		else {
			// This import is nested under another import
			auto imp = Context->ImportMap(-outerIdx - 1);
			parentStack.push(imp);
			outerIdx = imp.OuterIndex;
		}
	}

	bool isFirst = true;
	while (!parentStack.empty()) {
		FObjectResource entry = parentStack.top();
		parentStack.pop();
		if (isFirst) {
			isFirst = false;
			strcpy_s(cOutBuffer, entry.ObjectName.GetName());
		}
		else {
			strcat_s(cOutBuffer, ".");
			strcat_s(cOutBuffer, entry.ObjectName.GetName());
		}
	}

	return cOutBuffer;
}

UObject* CreateImport_hook(ULinkerLoad* Context, int i)
{
	UObject* object = CreateImport_orig(Context, i);
	if (object == nullptr)
	{
		auto filename = Context->Filename.Data;

		// Filter out startup files because they generate tons of imports that developers will never care about.
		if (wcsstr(filename, L"\\Startup_00_Shared") > 0
			|| wcsstr(filename, L"\\Startup_DLC_UNC_Moment01") > 0
			|| wcsstr(filename, L"\\Startup_HEN_VT") > 0
			|| wcsstr(filename, L"\\Startup_PRE_Cerberus") > 0
			|| wcsstr(filename, L"\\Startup_PRE_Collectors") > 0
			|| wcsstr(filename, L"\\Startup_PRE_DA") > 0
			|| wcsstr(filename, L"\\Startup_PRE_Terminus") > 0
			|| wcsstr(filename, L"\\Startup_PRE_General") > 0
			|| wcsstr(filename, L"\\Startup_PRO_Gulp01") > 0
			|| wcsstr(filename, L"\\Startup_PRO_Pepper01") > 0
			|| wcsstr(filename, L"\\Startup_PRO_Pepper02") > 0
			|| wcsstr(filename, L"\\Startup_DLC_UNC_Hammer01") > 0
			|| wcsstr(filename, L"\\Startup_Kasumi") > 0
			|| wcsstr(filename, L"\\Startup_CON_Pack01") > 0
			|| wcsstr(filename, L"\\Startup_MCR_03") > 0
			|| wcsstr(filename, L"\\Startup_UNC_Pack01") > 0
			|| wcsstr(filename, L"\\Startup_CER_02") > 0
			|| wcsstr(filename, L"\\Startup_Part01") > 0
			|| wcsstr(filename, L"\\Startup_DLC_DHME1") > 0
			|| wcsstr(filename, L"\\Startup_Pack02") > 0
			|| wcsstr(filename, L"\\Startup_UPD_Patch02") > 0
			|| wcsstr(filename, L"\\Startup_UPD_Patch03") > 0
			|| wcsstr(filename, L"\\Startup_METR_Patch01") > 0
			|| wcsstr(filename, L"\\Startup_EXP_Part01") > 0
			|| wcsstr(filename, L"\\Startup_EXP_Part02") > 0
			|| wcsstr(filename, L"\\Startup_CON_Pack02") > 0
			)
		{
			return object;
		}

		FObjectImport importEntry = Context->ImportMap(i);
		writeln("Could not resolve import #%d: %hs (%hs) in file: %s", -i - 1, GetFullImportName(Context, importEntry), importEntry.ClassName.GetName(), Context->Filename.Data);
		logger.writeWideLineToLog(wstring_format(L"Could not resolve #%d: %hs (%hs) in file: %s", -i - 1, GetFullImportName(Context, importEntry), importEntry.ClassName.GetName(), filename));
		logger.flush();
	}
	return object;
}

///===============================
/// METHOD IMPLEMENTATIONS
///===============================
void LogF_hook(void* fOutputDevice, wchar_t* formatStr, void* param1, void* param2)
{
	fwprintf_s(stdout, L"appLogf: ");
	fwprintf_s(stdout, formatStr, param1, param2);
	fwprintf_s(stdout, L"\n");
}

void MsgF_hook(void* fOutputDevice, wchar_t* formatStr, wchar_t* param1)
{
	fwprintf_s(stdout, L"appMsgf: ");
	fwprintf_s(stdout, formatStr, param1);
	fwprintf_s(stdout, L"\n");
}

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

#pragma region LogInternal
struct LE2FFrameHACK
{
	void* vtable; // 0x0
	int unknown[3];
	UStruct* Node;
	UObject* Object;
	BYTE* Code;
	// No idea what this other stuff is
};

typedef void (*tNativeFunction) (UObject* Context, LE2FFrameHACK* Stack, void* Result);
tNativeFunction* GNatives = (tNativeFunction*)0x7ff7c76d99b0; // Probably shouldn't hardcode this...

typedef void (*tLogInternalNative)(UObject* callingObject, LE2FFrameHACK* param2);
tLogInternalNative LogInternal = nullptr;
tLogInternalNative LogInternal_orig = nullptr;
void LogInternal_hook(UObject* callingObject, LE2FFrameHACK* stackFrame)
{
	// 0x20 = Object?
	// 0x28 = Code?
	// 0x2C = ??
	// 0x38 = PreviousFrame?
	auto name = callingObject->GetName();
	writeln(L"CALLING NATIVE LOGINTERNAL FOR %hs", name);
	std::this_thread::sleep_for(chrono::seconds(10)); // give time window to attach debugger to running process

	int64 posi = (int64) LogInternal_orig;
	for(int i = 0; i < 20; i++)
	{
		writeln(L"0x%p: %x", posi, *((BYTE*)posi));
		posi++;
	}

	BYTE* originalCodePointer = stackFrame->Code;
	BYTE nativeIndex = *stackFrame->Code++;
	FString stringArg;
	UObject* sfObject = stackFrame->Object;
	GNatives[nativeIndex](sfObject, (LE2FFrameHACK*)stackFrame, &stringArg);
	writeln(L"LogInternal() from %hs: %s", callingObject->GetFullName(), stringArg.Data);

	//restore the code pointer so LogInternal executes normally.
	stackFrame->Code = originalCodePointer;
	LogInternal_orig(callingObject, stackFrame);
}

#pragma endregion LogInternal


// ======================================
// LOGGING HOOKS
// ======================================
bool hookLoggingFunc(ISharedProxyInterface* InterfacePtr)
{
	// ============================================================
	// appLogF
	//=============================================================
	INIT_FIND_PATTERN_POSTHOOK(LogF,/*48 8b c4 48 89*/ "50 10 4c 89 40 18 4c 89 48 20 56 48 83 ec 50 83 79 08 00 48 8b f1 0f 85 bf 00 00 00");
	INIT_HOOK_PATTERN(LogF)

		/*
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
		}*/

		// ==============================================================
		// FConfig::CombineFromBuffer
		// ==============================================================

		INIT_FIND_PATTERN_POSTHOOK(FConfigCombineFromBuffer,/*40 55 56 57 41*/ "54 41 55 41 56 41 57 48 8d ac 24 b0 fe ff ff 48 81 ec 50 02 00 00 48 c7 85 d0 00 00 00 fe ff ff ff 48 89 9c 24 a8 02 00 00 48 8b 05 c3 c4 5f 01 48 33 c4 48 89 85 40 01 00 00");
		INIT_HOOK_PATTERN(FConfigCombineFromBuffer)

		//writeln(L"Locating FConfig::CombineFromBuffer...");
		//if (auto const rc = InterfacePtr->FindPattern(reinterpret_cast<void**>(&FConfigCombineFromBuffer), "40 55 56 57 41 54 41 55 41 56 41 57 48 8d ac 24 b0 fe ff ff 48 81 ec 50 02 00 00 48 c7 85 d0 00 00 00 fe ff ff ff 48 89 9c 24 a8 02 00 00 48 8b 05 c3 c4 5f 01 48 33 c4 48 89 85 40 01 00 00");
		//	rc != SPIReturn::Success)
		//{
		//	writeln(L"Failed to find FConfig::CombineFromBuffer pattern: %d / %s", rc, SPIReturnToString(rc));
		//	//return false;
		//}
		//else if (auto const rc = InterfacePtr->InstallHook(MYHOOK "FConfigCombineFromBuffer", FConfigCombineFromBuffer, FConfigCombineFromBuffer_hook, reinterpret_cast<void**>(&FConfigCombineFromBuffer_orig));
		//	rc != SPIReturn::Success)
		//{
		//	writeln(L"Attach - failed to hook FConfigCombineFromBuffer: %d / %s", rc, SPIReturnToString(rc));
		//	//return false;
		//}
		//else
		//{
		//	writeln(L"Hooked FConfigCombineFromBuffer");
		//}

		return true;
}

// This only works for printing native addresses
// Cannot have bink dll hook or it'll fail to bind

// A prototype of UFunction::Bind method used to
// bind UScript functions to native implementations.
typedef void(__thiscall* tUFunctionBind)(void* pFunction);
tUFunctionBind UFunctionBind = nullptr;
tUFunctionBind UFunctionBind_orig = nullptr;

set<void*> nativeSet;

void HookedUFunctionBind(UFunction* functionObj)
{
	UFunctionBind_orig(functionObj);
	if (functionObj->FunctionFlags & 0x400) { // NATIVE
		auto retVal = nativeSet.insert(functionObj->Func);
		if (retVal.second) {
			// Not really sure how to get the parent name...
			auto name = functionObj->GetFullName();
			auto finalAddr = (unsigned long long)functionObj->Func;
			writeln(L"%hs = 0x%p", name, finalAddr);
			logger.writeToLog(string_format("%s = 0x%p", name, finalAddr), false, true);
		}
	}
	else
	{
		//writeln(L"NOT NATIVE: %hs", functionObj->GetFullName());
	}
}

SPI_IMPLEMENT_ATTACH
{
	Common::OpenConsole();

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


	INIT_HOOK_PATTERN(OutputDebugStringW)

	auto _ = SDKInitializer::Instance();
	INIT_FIND_PATTERN_POSTHOOK(CreateImport, /*48 8b c4 55 41*/ "54 41 55 41 56 41 57 48 8b ec 48 83 ec 70 48 c7 45 d0 fe ff ff ff 48 89 58 10 48 89 70 18 48 89 78 20 4c 63 e2");
	INIT_HOOK_PATTERN(CreateImport);

	// RESEARCHING...
	INIT_FIND_PATTERN_POSTHOOK(LogInternal, /*40 57 48 83 ec*/ "40 48 c7 44 24 20 fe ff ff ff 48 89 5c 24 50 48 89 74 24 60 48 8b da 33 f6 48 89 74 24 28");
	INIT_HOOK_PATTERN(LogInternal);

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