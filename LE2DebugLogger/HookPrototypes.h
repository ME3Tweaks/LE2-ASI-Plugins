#pragma once

// Loading file to string
//typedef bool (*tappLoadFileToString)(FString* result, wchar_t* filename, void* fileManager, unsigned int flags1);
//tappLoadFileToString appLoadFileToString_orig = nullptr;
//tappLoadFileToString appLoadFileToString = nullptr;


// Creating an import
typedef UObject* (*tCreateImport)(ULinkerLoad* Context, int UIndex);
tCreateImport CreateImport = nullptr;
tCreateImport CreateImport_orig = nullptr;

// Load package persistent
//typedef UObject* (*tLoadPackagePersistent)(int64 param1, const wchar_t* param2, uint32 param3, int64* param4, uint32* param5);
//tLoadPackagePersistent LoadPackagePersistent = nullptr;
//tLoadPackagePersistent LoadPackagePersistent_orig = nullptr;

// In-game logging functions
// =========================================

// LogF
typedef void (*tFOutputDeviceLogF)(void* outputDevice, wchar_t* formatStr, void* param1, void* param2); // Used by multiple methods
tFOutputDeviceLogF LogF = nullptr;
tFOutputDeviceLogF LogF_orig = nullptr;

// LogErrorF
typedef void (*tFOutputDeviceErrorLogF)(void* outputDevice, int* code, wchar_t* formatStr, void* param1);
tFOutputDeviceErrorLogF ErrorLogF = nullptr;
tFOutputDeviceErrorLogF ErrorLogF_orig = nullptr;

tFOutputDeviceLogF MsgF = nullptr;
tFOutputDeviceLogF MsgF_orig = nullptr;

// Shows message to screen (disabled in release builds of game)
typedef void (*tMessageBoxF)(int messageBoxType, wchar_t* formatString, wchar_t* param3);
tMessageBoxF MsgFDialog = nullptr;
tMessageBoxF MsgFDialog_orig = nullptr;

// GAME INI FUNCTIONS
//===========================================

// Called when combining file from buffer
//typedef void (*tCombineFromBuffer)(void* Context, wchar_t* filePath, FString* contents, int extra);
//tCombineFromBuffer FConfigCombineFromBuffer = nullptr;
//tCombineFromBuffer FConfigCombineFromBuffer_orig = nullptr;


// MISC THINGS
// ==========================================

// Called when registering a TFC file
//typedef void (*tRegisterTFC)(FString* name);
//tRegisterTFC RegisterTFC = nullptr;
//tRegisterTFC RegisterTFC_orig = nullptr;

// Method signature for appFindFiles
//typedef void (*tFindFiles)(void* classPtr, TArray<wchar_t>* outFiles, wchar_t* searchPattern, bool files, bool directories, int flagSet);
//tFindFiles FindFiles = nullptr;
//tFindFiles FindFiles_orig = nullptr;