#pragma once

#include "../../Shared-ASI/Interface.h"
#include "../../Shared-ASI/Common.h"
#include "../../Shared-ASI/ME3Tweaks//ME3TweaksHeader.h"
#define MYHOOK "DebugLogger_"

// STRUCTS =====================================================
struct LE2FFrameHACK
{
	void* vtable; // 0x0
	int unknown[3];
	UStruct* Node;
	UObject* Object;
	BYTE* Code;
	// No idea what this other stuff is
};




struct FPackageFileCache;

typedef bool (*tFPackageFileCacheFindPackageFile) (wchar_t* packageName, FGuid* guid, FString& OutFileName, wchar_t* language, bool unknown);
typedef wchar_t** (*tFPackageFileCacheGetPackageFileList) (FPackageFileCache* cache, wchar_t* FileName);

// Used to lookup packages. Fully native
struct FPackageFileCache
{
    virtual void Unknown0() = 0;
    virtual void Unknown1() = 0;
    // Finds a package file by name
    virtual bool FindPackageFile(wchar_t* packageName, FGuid* guid, FString& OutFileName, wchar_t* language, bool unknown) = 0;
    // Returns list of packages. In shipping build of game this just says this method shouldn't be called
    virtual TArray<FString> GetPackageFileList(TArray<FString> outData) = 0;
    // Not sure on the parameters for this func. It's never called by game directly. Just prints out shouldn't happen in ship
    virtual TArray<FString> GetPackageNameList(TArray<FString> outData) = 0;
};


// The address at this location points to the static GPackageFileCache object pointer
FPackageFileCache* GPackageFileCache = nullptr;

// Used to lookup packages. Fully native
struct FFileManager
{
    virtual void Unknown0() = 0;
    virtual void Unknown1() = 0;
    virtual void Unknown2() = 0;
    virtual void Unknown3() = 0;
    virtual void Unknown4() = 0;
    virtual void Unknown5() = 0;
    virtual void Unknown6() = 0;
    virtual void Unknown7() = 0;
    virtual void Unknown8() = 0;
    virtual void Unknown9() = 0;
    virtual void Unknown10() = 0;
    virtual void Unknown11() = 0;
    virtual void Unknown12() = 0;
    virtual void Unknown13() = 0;
    virtual void Unknown14() = 0;
    virtual void Unknown15() = 0;
    virtual void Unknown16() = 0;
    virtual void Unknown17() = 0;
    virtual void Unknown18() = 0;
    virtual void Unknown19() = 0;
    virtual void Unknown20() = 0;
    virtual void Unknown21() = 0;
    virtual void Unknown22() = 0;
    virtual void Unknown23() = 0;
    virtual void Unknown24() = 0;
    virtual void Unknown25() = 0;
    virtual void Unknown26() = 0;
    virtual void Unknown27() = 0;
    virtual void Unknown28() = 0;
    virtual void Unknown29() = 0;

    virtual WCHAR** GetLoadingFileName(void* something) = 0;
};

FFileManager* GFileManager = nullptr;

// Used to load packages
struct UnLinker
{
    virtual void Unknown0() = 0;
    wchar_t* PackageName;
    //virtual void Unknown3() = 0;
    //virtual void Unknown4() = 0;
    //virtual void Unknown5() = 0;
    //virtual void Unknown6() = 0;
    //virtual void Unknown7() = 0;
    //virtual void Unknown8() = 0;
    //virtual void Unknown9() = 0;
    //virtual void Unknown10() = 0;
    //virtual void Unknown11() = 0;
    //virtual void Unknown12() = 0;
    //virtual void Unknown13() = 0;
    INT NameCount1; // IDK
    INT NameCount2; // IDK
    FGuid PackageGuid;
    ULinkerLoad* Linker;
    TArray<void*> CompletionCallbacks;
    INT ImportIndex;
    INT ExportIndex;
    INT PreLoadIndex;
    INT PostLoadIndex;
    FLOAT TimeLimit;
    BOOL bUseTimeLimit;
    BOOL bTimeLimitExceeded;
    DOUBLE TickStartTime;
    //UObject* LastObjectWorkWasPeformedOn;
    //TCHAR* LastTypeOfWorkPerformed;
    //DOUBLE LoadStartTime;
    //FLOAT LoadPercentage;
    //BOOL bHasFinishedExportGuids;

    FLOAT Load1;
    FLOAT Load2;
    FLOAT Load3;
    FLOAT Load4;
    FLOAT Load5;
    FLOAT Load6;
    FLOAT Load7;
    FLOAT EstimatedLoadPercentage;

};

typedef void (*tNativeFunction) (UObject* Context, LE2FFrameHACK* Stack, void* Result);
tNativeFunction* GNatives = nullptr;

// UTILITY METHODS ======================================================================

// Searches for the specified byte pattern, which is a 7-byte mov or lea instruction, with the 'source' operand being the address being calculated
void* findAddressLeaMov(ISharedProxyInterface* InterfacePtr, char* name, char* bytePattern)
{
    void* patternAddr;
    if (const auto rc = InterfacePtr->FindPattern(&patternAddr, bytePattern);
        rc != SPIReturn::Success)
    {
        writeln(L"Failed to find %hs pattern: %d / %s", name, rc, SPIReturnToString(rc));
        return nullptr; // Will be 0
    }

    // This is the address of the instruction
    const auto instruction = static_cast<BYTE*>(patternAddr);
    const auto RIPaddress = instruction + 7; // Relative Instruction Pointer (after instruction)
    const auto offset = *reinterpret_cast<int32_t*>(instruction + 3); // Offset listed in the instruction
    return RIPaddress + offset; // Added together we get the actual address
}

// Loads commonly used class pointers
void LoadCommonClassPointers(ISharedProxyInterface* InterfacePtr)
{
    // NEEDS UPDATED FOR LE2
    // 0x7ff74a9df82d DRM free | MOV RCX, qword ptr [GFileManager]
    auto addr = findAddressLeaMov(InterfacePtr, "GFileManager", "48 8b 0d 9c dd 67 01 ff d3 90 4c 89 6c 24 60 48 8b 4c 24 58");
    if (addr != nullptr)
    {
        GFileManager = static_cast<FFileManager*>(addr);
        writeln("Found GFileManager at %p", GFileManager);
    }
    else
    {
        writeln(" >> FAILED TO FIND GFileManager!");
    }

    // 0x7ff7c613b7e6 DRM free | MOV RAX, qword ptr [GPackageFileCache]
    addr = findAddressLeaMov(InterfacePtr, "GPackageFileCache", "48 8b 05 a3 70 5a 01 48 8b 08 48 8b 59 10 4d 85 f6 75 53");
    if (addr != nullptr)
    {
        GPackageFileCache = static_cast<FPackageFileCache*>(addr);
        writeln("Found GPackageFileCache at %p", GPackageFileCache);
    }
    else
    {
        writeln(" >> FAILED TO FIND GPackageFileCache!");
    }

    // 0x7ff7c60f0084 DRM free | LEA R9, [GNatives]
    addr = findAddressLeaMov(InterfacePtr, "GNatives", "4c 8d 0d 25 99 5e 01 4d 8b 0c c1 4c 8d 44 24 58");
    if (addr != nullptr)
    {
        GNatives = static_cast<tNativeFunction*>(addr);
        writeln("Found GNatives at %p", GNatives);
    } else
    {
        writeln(" >> FAILED TO FIND GNatives!");
    }
}

// HOOK SIGNATURES ====================================================

// Load file to string
typedef bool (*tappLoadFileToString)(FString* result, wchar_t* filename, void* fileManager, unsigned int flags1);
tappLoadFileToString appLoadFileToString_orig = nullptr;
tappLoadFileToString appLoadFileToString = nullptr;

// CreateImport
typedef UObject* (*tCreateImport)(ULinkerLoad* Context, int UIndex);
tCreateImport CreateImport = nullptr;
tCreateImport CreateImport_orig = nullptr;

typedef UObject* (*tCreateExport)(ULinkerLoad* Context, int UIndex);
tCreateExport CreateExport = nullptr;
tCreateExport CreateExport_orig = nullptr;

// ProcessEvent
typedef void (*tProcessEvent)(UObject* Context, UFunction* Function, void* Parms, void* Result);
tProcessEvent ProcessEvent = nullptr;
tProcessEvent ProcessEvent_orig = nullptr;

// Load package persistent (Not sure this is actually useful or correct)
//typedef UObject* (*tLoadPackagePersistent)(int64 param1, const wchar_t* param2, uint32 param3, int64* param4, uint32* param5);
//tLoadPackagePersistent LoadPackagePersistent = nullptr;
//tLoadPackagePersistent LoadPackagePersistent_orig = nullptr;

// LogF
typedef void (*tFOutputDeviceLogF)(void* outputDevice, wchar_t* formatStr, void* param1, void* param2); // Used by multiple methods
tFOutputDeviceLogF FOutputDeviceLogf = nullptr;
tFOutputDeviceLogF FOutputDeviceLogf_orig = nullptr;

// LogErrorF
typedef void (*tFOutputDeviceErrorLogF)(void* outputDevice, int* code, wchar_t* formatStr, void* param1);
tFOutputDeviceErrorLogF FErrorOutputDeviceLogf = nullptr;
tFOutputDeviceErrorLogF FErrorOutputDeviceLogf_orig = nullptr;

tFOutputDeviceLogF MsgF = nullptr;
tFOutputDeviceLogF MsgF_orig = nullptr;

// Shows message to screen (disabled in release builds of game)
typedef void (*tMessageBoxF)(int messageBoxType, wchar_t* formatString, wchar_t* param3);
tMessageBoxF MsgFDialog = nullptr;
tMessageBoxF MsgFDialog_orig = nullptr;

// Called when combining file from buffer
typedef void (*tCombineFromBuffer)(void* Context, wchar_t* filePath, FString* contents, int extra);
tCombineFromBuffer FConfigCombineFromBuffer = nullptr;
tCombineFromBuffer FConfigCombineFromBuffer_orig = nullptr;

typedef void* (*tLoadPackage)(void* param1, wchar_t* param2, uint32 param3);
tLoadPackage LoadPackage = nullptr;
tLoadPackage LoadPackage_orig = nullptr;

typedef uint32(*tAsyncLoadMethod)(UnLinker* linker, int a2, float a3);
tAsyncLoadMethod LoadPackageAsyncTick = nullptr;
tAsyncLoadMethod LoadPackageAsyncTick_orig = nullptr;

typedef void (*uLinkerPreload)(UnLinker* linker, UObject* objectToLoad);
uLinkerPreload LinkerLoadPreload = nullptr;
uLinkerPreload LinkerLoadPreload_orig = nullptr;

typedef UObject* (*tLoadPackagePersistent)(int64 param1, const wchar_t* param2, uint32 param3, int64* param4, uint32* param5);
tLoadPackagePersistent LoadPackagePersistent = nullptr;
tLoadPackagePersistent LoadPackagePersistent_orig = nullptr;

typedef void (*tLogInternalNative)(UObject* callingObject, LE2FFrameHACK* param2);
tLogInternalNative LogInternal = nullptr;
tLogInternalNative LogInternal_orig = nullptr;

// MISC THINGS
// ==========================================

// Called when registering a TFC file
typedef void (*tRegisterTFC)(FString* name);
tRegisterTFC RegisterTFC = nullptr;
tRegisterTFC RegisterTFC_orig = nullptr;

// Method signature for appFindFiles
typedef void (*tFindFiles)(void* classPtr, TArray<wchar_t>* outFiles, wchar_t* searchPattern, bool files, bool directories, int flagSet);
tFindFiles FindFiles = nullptr;
tFindFiles FindFiles_orig = nullptr;

// DebugLogger stuff for devs
// ==========================================
typedef UObject* (*tStaticAllocateObject)(
    UClass* objectClass, // What class of object is being instantiated?
    UObject* inObject, // The 'Outer' of the object will be set to this 
    FName a3, // Name of object?
    long long loadFlags,
    void* a5, // often 0
    void* errorDevice, //Often GError
    const wchar_t* a7, // Often 0
    void* a8, // Often 0
    void* a9); // Often 0
tStaticAllocateObject StaticAllocateObject = nullptr;
tStaticAllocateObject StaticAllocateObject_orig = nullptr;

