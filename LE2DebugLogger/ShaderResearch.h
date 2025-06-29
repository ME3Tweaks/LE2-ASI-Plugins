#pragma once
#include <chrono>
#include <thread>
#include <set>
#include <map>
#include "../../Shared-ASI/Interface.h"

// This file self contains shader binding research code.

// Prints out ghidra addresses for import
#define PRINT_FOR_GHIDRA FALSE
// Prints out binding information
#define PRINT_BINDING_INFO !PRINT_FOR_GHIDRA && TRUE // Set to false to fully disable this.
// Prints out serialization info
#define PRINT_SERIALIZATION_INFO !PRINT_FOR_GHIDRA && TRUE // Set to false to fully disable this.


// ===============================================================================================
// Macros
// ===============================================================================================
// Used to define a struct binding setup
#define IMPLEMENT_STRUCT_BINDING_HOOK(structName) \
	tSubStructBind structName##Bind = nullptr; \
	tSubStructBind structName##Bind_orig = nullptr; \
	void structName##Bind_hook(void* structAddr, FShaderParameterMap* Map) { \
		HookedStructBinding(structAddr, Map, L#structName, structName##Bind_orig); \
	} \

// Used to define a struct binding setup (3 parameter version)
#define IMPLEMENT_STRUCT_BINDING_HOOK3(structName) \
	tSubStruct3Bind structName##Bind = nullptr; \
	tSubStruct3Bind structName##Bind_orig = nullptr; \
	void structName##Bind_hook(void* structAddr, void* unk2, FShaderParameterMap* Map) { \
		HookedStructBinding(structAddr, unk2, Map, L#structName, structName##Bind_orig); \
	} \

#define HOOK_STRUCT_BINDING(structName, rva) { \
	structName##Bind = (tSubStructBind)SDKInitializer::Instance()->GetAbsoluteAddress(rva); \
	if (SPIReturn rc = ISharedProxyInterface::SPIInterfacePtr->InstallHook(#structName "Bind", structName##Bind, structName##Bind_hook, (void**)&structName##Bind_orig); rc != SPIReturn::Success) { \
		std::cout << "Failed to hook " << #structName << "::Bind: " << SPIReturnToString(rc) << std::endl; \
	} \
} \

#define HOOK_STRUCT_BINDING3(structName, rva) { \
	structName##Bind = (tSubStruct3Bind)SDKInitializer::Instance()->GetAbsoluteAddress(rva); \
	if (SPIReturn rc = ISharedProxyInterface::SPIInterfacePtr->InstallHook(#structName "Bind", structName##Bind, structName##Bind_hook, (void**)&structName##Bind_orig); rc != SPIReturn::Success) { \
		std::wcout << "Failed to hook " << #structName << "::Bind: " << SPIReturnToString(rc) << std::endl; \
	} \
} \

// ===============================================================================================
// TypeDefs
// ===============================================================================================
typedef void (*tFShaderParameterBind)(void*, void*, wchar_t*, BOOL);
typedef TLinkedList<FShaderTypeSub*>** (*tGetShaderTypeList)();
typedef TLinkedList<FVertexFactoryType*>** (*tGetVertexFactoryTypeList)();
typedef void (*tSubStructBind)(void*, FShaderParameterMap*);
// Param 2 might be the construct compiled data?
typedef void (*tSubStruct3Bind)(void*, void*, FShaderParameterMap*);
typedef FArchive* (*tFShaderSerialize)(FShader*, FArchive*, void*, void*);
typedef void (*tDeregisterShader)(FShaderType*, FShader*);
typedef FArchive* (*tFShaderParameterSerialize)(FArchive*, void*);
typedef FArchive* (*tFVertexFactoryParameterRefSerialize)(FArchive*, FVertexFactoryParameterRef*);
typedef FShader* (*tFShaderConstructFromCompiled)(FShader*, CompiledShaderData*, void*, void*);
// For creating archive proxies
typedef FArchiveProxy* (*tCreateArchiveProxy)(FArchiveProxy* self, FArchive* arcToProxy);
typedef void (*tSFXNameConstructor)(FName* outValue, const wchar_t* nameValue, int nameNumber, BOOL createIfNotFoundMaybe, BOOL unk2);
typedef void (*tShaderSerialize)(FShader*, FArchive*);
typedef void (*tSerializeGlobalShaders)(EShaderPlatform, FArchive*);

// ===============================================================================================
// Globals
// ===============================================================================================
// Until true, shader serialization will stall.
bool AllHooksInstalled = false;
// If we're recording binding names and offsets.
bool IsRecordingBinding = false;
// If we're recording a binding that isn't in already in a struct.
bool IsRecordingBaseBinding = false;
// If we're mapping serialization offsets to the binding offsets.
bool IsRecordingSerialization = false;

// ===============================================================================================
// HOOKING VARIABLES
// ===============================================================================================
// Used by both parameter types.
tFShaderParameterBind FShaderParameterBind = nullptr;
tFShaderParameterBind FShaderParameterBind_orig = nullptr;
tFShaderParameterBind FShaderResourceParameterBind = nullptr;
tFShaderParameterBind FShaderResourceParameterBind_orig = nullptr;
tGetShaderTypeList GetShaderTypeList = nullptr; // @ 0x253cc0 LE1
tGetVertexFactoryTypeList GetVertexFactoryTypeList = nullptr; // @ 0x62b940 LE1 
tFShaderSerialize FShaderSerialize = nullptr;
tFShaderSerialize FShaderSerialize_orig = nullptr;
// Not hooked but invokable
tDeregisterShader DeregisterShader = nullptr;
tFShaderParameterSerialize FShaderParameterSerialize = nullptr;
tFShaderParameterSerialize FShaderParameterSerialize_orig = nullptr;
tFShaderParameterSerialize FShaderResourceParameterSerialize = nullptr;
tFShaderParameterSerialize FShaderResourceParameterSerialize_orig = nullptr;
tFVertexFactoryParameterRefSerialize FVertexFactoryParameterRefSerialize = nullptr;
tFVertexFactoryParameterRefSerialize FVertexFactoryParameterRefSerialize_orig = nullptr;
tFShaderConstructFromCompiled FShaderConstructFromCompiled = nullptr;
tFShaderConstructFromCompiled FShaderConstructFromCompiled_orig = nullptr;
tCreateArchiveProxy CreateArchiveProxy = nullptr;
tSFXNameConstructor _CreateName = nullptr;
tSerializeGlobalShaders SerializeGlobalShaders = nullptr;
tSerializeGlobalShaders SerializeGlobalShaders_orig = nullptr;

// ===============================================================================================
// VTables
// ===============================================================================================

void* MemoryArchiveReaderVTableStart = nullptr;
void* MemoryArchiveWriterVTableStart = nullptr;
void* FileReaderVTableStart = nullptr;
void* StringArchiveProxyVTableStart = nullptr;

void SetVTable(void** objectStartAddress, void* newVTableStartAddress) {
	*objectStartAddress = newVTableStartAddress; // Set the vtable pointer to the new table
}

void* GetVTable(void** objectStartAddress) {
	return *objectStartAddress;
}

// ===============================================================================================
// CUSTOM OBJECTS, UTILITY METHODS
// ===============================================================================================
// Type of the parameter that is bound/serialized.
enum EShaderParameterType : int {
	SPT_Normal = 0,
	SPT_Resource = 1,
	SPT_VertexFactory = 2,
	SPT_ParameterStruct = 3
};

std::wstring GetParameterTypeString(EShaderParameterType type) {
	switch (type) {
	case SPT_Normal:
		return L"Parameter";
	case SPT_Resource:
		return L"ResourceParameter";
	case SPT_VertexFactory:
		return L"VertexFactoryParameter";
	case SPT_ParameterStruct:
		return L"ParameterStruct";
	default:
		return L"Unknown";
	}
}

void CreateName(FName* outName, const wchar_t* nameValue, int num = 0) {
	_CreateName(outName, nameValue, num, TRUE, 0);
}

// ===============================================================================================
// BINDING NAME RECORDING =======================================================================
// ===============================================================================================

// BINDING GLOBALS ======================
// Prefixes output to denote sublevels of structs.
std::wstring RecordingPrefix(L"  ");
// The current deserialized shader
FShader* DeserializedShader = nullptr;
// The address we base relative addressing off, as the game
// malloc'd it
void* BindingBaseShaderAddress = nullptr;
// A dummy uniform expression set that we only allocate once and feed into compiled
// construtor. It doesn't have any data.
FUniformExpressionSet DummySet{};
// Map of Shader Type -> Map of Address->Type/Name combo.
// Map
//   ShaderType -> Map 
//                    Offset -> Type, ParameterName
std::map<std::wstring, std::map<DWORD, std::tuple<EShaderParameterType, std::wstring>>> bindingMap;

// Map of struct to the size of the struct. Used for skipping deserialization as it goes in order... I hope.
std::map<std::wstring, DWORD> structSizeMap;

bool IsRecordingSubStructSize = false;
DWORD NumBoundParametersInStruct = 0;
// Used for tripping breakpoints
bool breakPoint = false;

// BINDING METHODS AND HOOKS =================
void RecordBinding(void* varAddr, EShaderParameterType type, std::wstring parameterName) {
	if (IsRecordingBinding) {
		auto offset = (long long)varAddr - (long long)BindingBaseShaderAddress;

		if (IsRecordingSubStructSize)
		{
			if (type == SPT_Normal || type == SPT_Resource)
			{
				// Add a parameter to the count.
				NumBoundParametersInStruct++;
			}
		}

		if (IsRecordingBaseBinding && PRINT_BINDING_INFO) {
			std::wcout << std::hex << L"  @@ 0x" << offset << L"  " << RecordingPrefix.c_str() << GetParameterTypeString(type) << L"  " << parameterName << std::endl;
		}

		auto shaderMap = bindingMap.find(DeserializedShader->Type->Name);
		if (shaderMap != bindingMap.end()) {
			shaderMap->second.emplace(std::make_pair(offset, std::make_tuple(type, parameterName))); // param size is 6, callers will change this if necessary
		}
		else {
			// Uh oh.
			std::cout << "Uh oh.\n";
		}
	}
}

void RecordStructBinding(void* structAddr, std::wstring structName) {
	RecordBinding(structAddr, SPT_ParameterStruct, structName);
}


void FShaderParameterBind_hook(void* varAddr, void* parameterMap, wchar_t* parameterName, BOOL isOptional) {
	RecordBinding(varAddr, SPT_Normal, parameterName);
	// FShaderResourceParameterBind_orig(varAddr, parameterMap, parameterName, TRUE); // true suppresses messages
}

void FShaderResourceParameterBind_hook(void* varAddr, void* parameterMap, wchar_t* parameterName, BOOL isOptional) {
	RecordBinding(varAddr, SPT_Resource, parameterName);
	// FShaderResourceParameterBind_orig(varAddr, parameterMap, parameterName, TRUE); // true suppresses messages
}

// Shared logic version of struct hook
void HookedStructBinding(void* structAddr, FShaderParameterMap* Map, std::wstring structName, tSubStructBind bindFunc) {
	if (IsRecordingBinding) {
		if (IsRecordingBaseBinding) {
			// Only record bindings if we are at the base
			IsRecordingSubStructSize = true;
			NumBoundParametersInStruct = 0;
			RecordStructBinding(structAddr, structName);
		}

		// Cache entry values
		auto originalBaseBinding = IsRecordingBaseBinding;
		auto originalPrefix = RecordingPrefix;

		// We are now going to enter the struct, so we are no longer at the base
		// Update the recording prefix
		IsRecordingBaseBinding = false;
		std::wstring addlPrefix = L"[" + structName + L"]";
		RecordingPrefix = RecordingPrefix.append(addlPrefix);
		bindFunc(structAddr, Map);

		// Restore the original values
		RecordingPrefix = originalPrefix;
		IsRecordingBaseBinding = originalBaseBinding;

		if (originalBaseBinding && IsRecordingSubStructSize) {
			// Update the struct size for this binding. They should all be the same, but this is just easier... I think
			// std::cout << "---------- Struct size (per binding): 0x" << std::hex << (NumBoundParametersInStruct * 6) << std::endl;
			structSizeMap.try_emplace(structName, NumBoundParametersInStruct * 6);
			NumBoundParametersInStruct = 0;
			IsRecordingSubStructSize = false;
		}
	}
	else
	{
		bindFunc(structAddr, Map);
	}
}


// Shared logic version of struct hook (3 parameter version)
void HookedStructBinding(void* structAddr, void* unk2, FShaderParameterMap* Map, std::wstring structName, tSubStruct3Bind bindFunc) {
	if (IsRecordingBinding) {
		if (IsRecordingBaseBinding) {
			// Only record bindings if we are at the base
			RecordStructBinding(structAddr, structName);
		}

		// Cache entry values
		auto originalBaseBinding = IsRecordingBaseBinding;
		auto originalPrefix = RecordingPrefix;

		// We are now going to enter the struct, so we are no longer at the base
		// Update the recording prefix
		IsRecordingBaseBinding = false;
		std::wstring addlPrefix = L"[" + structName + L"]";
		RecordingPrefix = RecordingPrefix.append(addlPrefix);

		bindFunc(structAddr, unk2, Map);
		// Restore the original values
		RecordingPrefix = originalPrefix;
		IsRecordingBaseBinding = originalBaseBinding;
	}
	else
	{
		bindFunc(structAddr, unk2, Map);
	}
}

// Parameter struct types
// Some of these might be unused.
IMPLEMENT_STRUCT_BINDING_HOOK(FSceneTextureShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FLightShaftPixelShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FDOFShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK3(FMaterialPixelShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FSpotLightPolicy_ModShadowPixelParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FAmbientOcclusionParameters);
IMPLEMENT_STRUCT_BINDING_HOOK3(FMaterialBaseShaderParameters); // Need to see if this is in LE2.
IMPLEMENT_STRUCT_BINDING_HOOK(FFogVolumeVertexShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FHeightFogVertexShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FDirectionalLightPolicyPixelParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FHBAOShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FGammaShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FColorRemapShaderParameters);
IMPLEMENT_STRUCT_BINDING_HOOK(FMaterialVertexShaderParameters);

// LE2
IMPLEMENT_STRUCT_BINDING_HOOK(FSpotlightPolicy_ModShadowPixelParamsType);


FShader* FShaderConstructFromCompiled_hook(FShader* inShader, CompiledShaderData* compiledInfo, void* unk3, void* unk4)
{
	// This method normally sets up a freshly malloc'd shader object (inShader) from the compiledInfo object.
	// We don't care about any of that.
	if (IsRecordingBinding) {
		// Record the address of the passed in shader object, as all binding operations are relative to this address.
		BindingBaseShaderAddress = inShader;
		return inShader;
	}
	else
	{
		return FShaderConstructFromCompiled_orig(inShader, compiledInfo, unk3, unk4);
	}
}

// ===============================================================================================
// SERIALIZATION RECORDING =======================================================================
// ===============================================================================================

// Global ==================
// The archive offset in which we base our file serialialization offsets from.
DWORD SerializationBaseOffset = 0;
// Used to skip printing out substruct serializations
DWORD SkipUntilOffset = 0;
// If we are binding the base parameters and not a substruct.
bool IsSerializingBase = true;
// Map of shader names to their serialization functions.
// Original VTable -> Serialization func. We can't trust the vtable offset to serialize, as the serialize function is hooked and will have changed.
std::map<void*, tShaderSerialize> SerializationFunctionMap;
// ShaderType Name-> OriginalSerializationFunction
// Note the detour function is all the same so we don't really care.
//std::map<std::wstring, tShaderSerialize> SerializationFunctionMap;

// Functions ===============
void PrintSerialization(DWORD ArOffset, EShaderParameterType type, void* paramAddr) {
	auto serializationOffset = ArOffset - SerializationBaseOffset;
	if (serializationOffset < 0) {
		std::cout << "ya done goofd\n";
	}
	if (SkipUntilOffset > serializationOffset) {
		return;
	}

	// Reset this here
	SkipUntilOffset = 0;

	auto mapPair = bindingMap.find(DeserializedShader->Type->Name);
	if (mapPair != bindingMap.end()) {
		auto map = mapPair->second;
		DWORD paramOffset = (long long)paramAddr - (long long)DeserializedShader; // Compute from the address of the shader object
		if ((int)paramOffset < 0) {
			std::cout << "ya done goofd again\n";
		}
		auto bindingOffsetInfo = map.find(paramOffset);
		if (bindingOffsetInfo == map.end()) {
			// Not bound or vertex factory params (haven't figured those out yet.)
			if (type == SPT_VertexFactory) {
				auto vfParam = (FVertexFactoryParameterRef*)paramAddr;
				auto vfType = vfParam->VertexFactoryType;
				if (vfType == nullptr) {
					std::wcout << std::hex << L"  << 0x" << serializationOffset << L" " << GetParameterTypeString(type) << L"  (NULL VF)" << std::endl;
				}
				else {
					std::wcout << std::hex << L"  << 0x" << serializationOffset << L" " << GetParameterTypeString(type) << L"  " << vfParam->VertexFactoryType->Name << L"Params" << std::endl;
				}
			}
			else {
				// Binding - not found!
				std::wcout << std::hex << L"  << 0x" << serializationOffset << L" " << GetParameterTypeString(type) << L"  UNBOUND (@@ 0x" << paramOffset << L")" << std::endl;
			}
		}
		else {
			// We found a matching bind point
			auto [paramType, name] = bindingOffsetInfo->second;
			std::wcout << std::hex << L"  << 0x" << serializationOffset << L" " << GetParameterTypeString(type) << L"  " << name.c_str() << std::endl;

			if (paramType == SPT_ParameterStruct) {
				// See if we have the size for this...
				auto sizePair = structSizeMap.find(name);
				if (sizePair != structSizeMap.end()) {
					SkipUntilOffset = serializationOffset + sizePair->second;
				}
			}
		}
	}
}

FArchive* FShaderParameterSerialize_hook(FArchive* ar, void* param) {
	DWORD arOffset = ar->VTable->Tell(ar);
	// Serialization may populate param with info we need
	auto result = FShaderParameterSerialize_orig(ar, param);
	if (IsRecordingSerialization && PRINT_SERIALIZATION_INFO) {

		PrintSerialization(arOffset, SPT_Normal, param);
	}
	return result;
}

FArchive* FShaderResourceParameterSerialize_hook(FArchive* ar, void* param) {
	DWORD arOffset = ar->VTable->Tell(ar);
	// Serialization may populate param with info we need
	auto result = FShaderResourceParameterSerialize_orig(ar, param);
	if (IsRecordingSerialization && PRINT_SERIALIZATION_INFO) {

		PrintSerialization(arOffset, SPT_Resource, param);
	}
	return result;
}

FArchive* FVertexFactoryParameterRefSerialize_hook(FArchive* ar, FVertexFactoryParameterRef* param) {
	DWORD arOffset = ar->VTable->Tell(ar);
	auto result = FVertexFactoryParameterRefSerialize_orig(ar, param);
	if (IsRecordingSerialization && PRINT_SERIALIZATION_INFO) {
		PrintSerialization(arOffset, SPT_VertexFactory, param);
	}
	return result;
}



// ================================================================================================
// SHARED CODE
// =================================================================================================

// Invokes the original serialization code for an FShader.
void InvokeOriginalShaderSerialization(FShader* shader, FArchive* ar) {
	auto vtable = GetVTable((void**)shader);
	auto originalFunc = SerializationFunctionMap.find(vtable);
	if (originalFunc != SerializationFunctionMap.end()) {
		// Invoke the original serialization function
		originalFunc->second(shader, ar);
	}
	else {
		std::cout << "Serious uh ohs have occurred.\n";
	}
}

// This marks the start of the serialization of an FShader.
void CombinedShaderSerialize_hook(FShader* shader, FArchive* ar) {
	// Record the shader object and serialization start.
	DeserializedShader = shader;
	SerializationBaseOffset = ar->VTable->Tell(ar);

	// Reset variables.
	IsRecordingBinding = false;
	IsRecordingSerialization = false;

	// For us to get the type, we unforutnately have to serialize the shader, cause even though the calling func could have
	// set it, it didn't.
	InvokeOriginalShaderSerialization(shader, ar);
	auto shaderType = shader->Type;
	DeregisterShader(shaderType, shader); // Get rid of it

	// Rewind
	ar->VTable->Seek(ar, SerializationBaseOffset);


	auto neverSeenShader = bindingMap.find(shader->Type->Name) == bindingMap.end();
	if (!neverSeenShader) {
		// We've seen this shader type, just serialize it.
		InvokeOriginalShaderSerialization(shader, ar);
		return;
	}

	// Encountered a new shader type


	// Binding recording.
	IsRecordingBinding = true;
	IsRecordingBaseBinding = true;

	if (PRINT_BINDING_INFO) {
		std::wcout << shader->Type->Name << std::endl;
	}

	// Assign this here, we will access it later in the hooks
	DeserializedShader = shader;

	// Step 1: Find the bind points for the shader.
	// We do a fake construct from compiled call, as we have hooked the bind methods.
	// We then record the calls to bind, calculating the offset from the address we record when
	// ConstructFromCompiled is called and the FShader is malloc'd.

	// Create the empty binding list
	std::wstring name = shader->Type->Name;
	auto map = std::map<DWORD, std::tuple<EShaderParameterType, std::wstring>>();
	bindingMap.emplace(name, map);

	VertexCompiledShaderData compiledInfo{};
	FShaderParameterMap parameterMap{}; // empty parameter map. Don't care about uniform expressions.
	// Just kidding. A few shaders need it for who knows why.
	parameterMap.UniformExpressionSet = &DummySet;
	compiledInfo.ParameterMap = &parameterMap;

	// LE2 ---
	// LE2 seems to access the code array, while LE1 does not.
	auto malloced = UnrealMalloc::GMalloc.Malloc(sizeof(TArray<BYTE>));
	memset(malloced, 0, sizeof(TArray<BYTE>));
	compiledInfo.Code = (TArray<BYTE>*) malloced;
	
	// LE2 also seems to need ShaderType...?
	compiledInfo.ShaderType = shader->Type;

	// End LE2 --
	if (shader->Target.Frequency == SF_VERTEX) {
		// Vertex shaders need a vertex factory, not sure if it matters which one.
		// Don't know if we need to properly enumerate these...........
		compiledInfo.VertexFactoryType = (*GetVertexFactoryTypeList())->CurrentItem;
	}
	int unk2 = 0;
	int unk3 = 0;
	int unk4 = 0;

	if (PRINT_FOR_GHIDRA) {

		// Print constructor for deserialization (makes blank object)
		std::wcout << shader->Type->Name << L"::ConstructForDeserialization 0x" << std::hex << shader->Type->ConstructSerializedRef << L" f" << std::endl;

		// Print constructor with binding addresses
		//std::wcout << shader->Type->Name << L"::ConstructFromCompiled 0x" << std::hex << shader->Type->ConstructCompiledRef << L" f" << std::endl;

		// Print Serialization Method addresses
		//auto vtable = GetVTable((void**)shader);
		//auto originalFunc = SerializationFunctionMap.find(vtable);
		//if (originalFunc != SerializationFunctionMap.end()) {
		//	// Invoke the original serialization function
		//	std::wcout << shader->Type->Name << L"::Serialize 0x" << std::hex << originalFunc->second << L" f" << std::endl;
		//}
		//else {
		//	std::cout << "Serious uh ohs have occurred.\n";
		//}
	}
	else
	{
		// This will invoke the ::Bind hooks
		shader->Type->ConstructCompiledRef((CompiledShaderData*)&compiledInfo, &unk2, &unk3, &unk4);
	}

	// Serialization recording.
	// STEP 2:
	// We should now have set up the binding map that will let us map serialization order.
	// We do not set this to false on exit, only on entry, as we want to record the serialization that occurs
	// in this function.
	IsRecordingSerialization = true;

	// We now use this as our base shader adddress as we are going back to the archive for serialization.
	DeserializedShader = shader; // reset this back.

	// Begin serialization, this will record the parameter serialization calls
	InvokeOriginalShaderSerialization(shader, ar);


	// Ensure we have reset this if the last paramter type was a struct.
	SkipUntilOffset = 0;

	// We are no longer recording bindings.
	IsRecordingBinding = false;
	// We also are no longer recording serializations.
	IsRecordingSerialization = false;
}

std::wstring SMToString(EShaderPlatform platform) {
	switch (platform) {
	case SP_360:
		return L"Xbox 360";
	case SP_PS3:
		return L"PS3";
	case SP_SM2: return L"PC SM2";
	case SP_SM3: return L"PC SM3";
	case SP_SM4: return L"PC SM4";
	case SP_SM5: return L"PC SM5";
	case SP_Dingo:
		return L"Xbox One";
	case SP_Orbis:
		return L"PS4";
	default:
		return L"Unknown";
	}
}

void HookShaderSerialize(void* vtable) {
	void** serializeFunc = (void**)((long long)vtable + (0x8 * sizeof(void*)));
	tShaderSerialize originalFun = (tShaderSerialize)*serializeFunc;

	// Write the hook
	DWORD flOldProtect = 0u;

	// Allow us to write to the vtable entry
	VirtualProtect(serializeFunc, sizeof(tShaderSerialize), PAGE_READWRITE, &flOldProtect);

	// Do your stuff
	*serializeFunc = (void*)CombinedShaderSerialize_hook;

	// Restore it
	VirtualProtect(serializeFunc, sizeof(tShaderSerialize), flOldProtect, &flOldProtect);

	// Record the original function so we can invoke it later
	SerializationFunctionMap.emplace(vtable, originalFun);
}

void hookAllShaderSerializationMethods() {
	int numDone = 0;
	//std::cout << "Hooking shader serialization methods...\n";

	auto shaderTypeListPtr = GetShaderTypeList();
	auto shaderTypeList = *shaderTypeListPtr;
	auto numToDo = shaderTypeList->Count();
	while (shaderTypeList != nullptr) {
		std::cout << "\rHooking shader serialization methods " << numDone << "/" << numToDo;

		auto shaderName = ws2s(shaderTypeList->CurrentItem->Name);
		shaderName.append("_Hook");

		// Construct a type of the shader; so we can get its vtable
		FShader* temp = shaderTypeList->CurrentItem->ConstructSerializedRef(shaderTypeList->CurrentItem);
		//if (temp->Target.Platform != SP_SM5) {
		//	// GlobalShaderCache is weird and has a bunch of shaders that aren't for SM5
		//	// No clue why they're there. they can't be used.
		//	std::wcout << L"Skipping non SM5 shader type " << shaderTypeList->CurrentItem->Name << L", it's for " << SMToString((EShaderPlatform)temp->Target.Platform) << std::endl;
		//	shaderTypeList = shaderTypeList->NextItem;
		//	numDone++;
		//	continue;
		//}

		auto vtable = GetVTable((void**)temp);
		HookShaderSerialize(vtable);
		shaderTypeList = shaderTypeList->NextItem;
		numDone++;
	}
	std::cout << std::endl;
}

void SerializeGlobalShaders_hook(EShaderPlatform platform, FArchive* Ar) {
	std::cout << "Installing hooks...\n";

	// This method is only ever called once VERY early in game boot
	while (AllHooksInstalled == false) {
		// Stall for hooks to install
		std::this_thread::sleep_for(200ms);
	}

	if (SerializationFunctionMap.size() == 0) {
		// Enumerate the shader types and hook all of their serialization functions so we can intercept the entry 
		// to the method.
		hookAllShaderSerializationMethods();
	}

	std::cout << "Done.\n";
	SerializeGlobalShaders_orig(platform, Ar);
}

// ================================================================================================
// ASI INITIALIZATION
// ================================================================================================
// This method is invoke on attach, set up your hooks here.
// THIS ASI DOES NOT WORK IF OTHER ASIS HOOK THE SAME FUNCTIONS, WE DON'T DO POSTHOOK. At least I think
// the SPI system doesn't know how to hook a hook
bool hookShaderResearch(ISharedProxyInterface* InterfacePtr) {

	SPIReturn result = SPIReturn::Success;

	// PUT THIS FIRST AS SHADER SERIALIZATION HAPPENS EARLY!
	SerializeGlobalShaders = (tSerializeGlobalShaders)SDKInitializer::Instance()->GetAbsoluteAddress(0x23c850); // LE2
	result = InterfacePtr->InstallHook("SerializeGlobalShaders", SerializeGlobalShaders, SerializeGlobalShaders_hook, (void**)&SerializeGlobalShaders_orig);


	//FShaderSerialize = (tFShaderSerialize)SDKInitializer::Instance()->GetAbsoluteAddress(0x259b20); // LE1
	//InterfacePtr->InstallHook("FShaderSerialize", FShaderSerialize, FShaderSerialize_hook, (void**)&FShaderSerialize_orig);

	DeregisterShader = (tDeregisterShader)SDKInitializer::Instance()->GetAbsoluteAddress(0x356f80); // LE2

	/** SERIALIZATION */
	FShaderParameterSerialize = (tFShaderParameterSerialize)SDKInitializer::Instance()->GetAbsoluteAddress(0x351220); // LE2
	result = InterfacePtr->InstallHook("FShaderParameterSerialize", FShaderParameterSerialize, FShaderParameterSerialize_hook, (void**)&FShaderParameterSerialize_orig);
	if (result != SPIReturn::Success) { std::cout << "Failed to install FShaderParameterSerialize hook: " << SPIReturnToString(result) << std::endl; }

	FShaderResourceParameterSerialize = (tFShaderParameterSerialize)SDKInitializer::Instance()->GetAbsoluteAddress(0x351270); // LE2
	result = InterfacePtr->InstallHook("FShaderResourceParameterSerialize", FShaderResourceParameterSerialize, FShaderResourceParameterSerialize_hook, (void**)&FShaderResourceParameterSerialize_orig);
	if (result != SPIReturn::Success) { std::cout << "Failed to install FShaderResourceParameterSerialize hook: " << SPIReturnToString(result) << std::endl; }

	FVertexFactoryParameterRefSerialize = (tFVertexFactoryParameterRefSerialize)SDKInitializer::Instance()->GetAbsoluteAddress(0x8d9fb0); // LE1
	result = InterfacePtr->InstallHook("FVertexFactoryParameterRefSerialize", FVertexFactoryParameterRefSerialize, FVertexFactoryParameterRefSerialize_hook, (void**)&FVertexFactoryParameterRefSerialize_orig);
	if (result != SPIReturn::Success) { std::cout << "Failed to install FVertexFactoryParameterRefSerialize hook: " << SPIReturnToString(result) << std::endl; }

	/** BINDS */
	FShaderConstructFromCompiled = (tFShaderConstructFromCompiled)SDKInitializer::Instance()->GetAbsoluteAddress(0x34e010); // LE2
	result = InterfacePtr->InstallHook("FShaderConstructFromCompiled", FShaderConstructFromCompiled, FShaderConstructFromCompiled_hook, (void**)&FShaderConstructFromCompiled_orig);
	if (result != SPIReturn::Success) { std::cout << "Failed to install FShaderConstructFromCompiled hook: " << SPIReturnToString(result) << std::endl; }

	FShaderParameterBind = (tFShaderParameterBind)SDKInitializer::Instance()->GetAbsoluteAddress(0x355300); // LE2
	result = InterfacePtr->InstallHook("FShaderParameterBind", FShaderParameterBind, FShaderParameterBind_hook, (void**)&FShaderParameterBind_orig);
	if (result != SPIReturn::Success) { std::cout << "Failed to install FShaderParameterBind hook: " << SPIReturnToString(result) << std::endl; }

	FShaderResourceParameterBind = (tFShaderParameterBind)SDKInitializer::Instance()->GetAbsoluteAddress(0x355390); // LE2
	result = InterfacePtr->InstallHook("FShaderResourceParameterBind", FShaderResourceParameterBind, FShaderResourceParameterBind_hook, (void**)&FShaderResourceParameterBind_orig);
	if (result != SPIReturn::Success) { std::cout << "Failed to install FShaderResourceParameterBind hook: " << SPIReturnToString(result) << std::endl; }



	HOOK_STRUCT_BINDING(FSceneTextureShaderParameters, 0x354dc0);
	HOOK_STRUCT_BINDING(FLightShaftPixelShaderParameters, 0x25a220);
	HOOK_STRUCT_BINDING(FDOFShaderParameters, 0x1e2c30);
	HOOK_STRUCT_BINDING3(FMaterialPixelShaderParameters, 0x28d340);
	//FSpotLightPolicy_ModShadowPixelParamsTypeBind = (tSubStructBind)SDKInitializer::Instance()->GetAbsoluteAddress(0x263950); // LE1
	//InterfacePtr->InstallHook("FSpotLightPolicy_ModShadowPixelParamsTypeBind", FSpotLightPolicy_ModShadowPixelParamsTypeBind, FSpotLightPolicy_ModShadowPixelParamsTypeBind_hook, (void**)&FSpotLightPolicy_ModShadowPixelParamsTypeBind_orig);
	HOOK_STRUCT_BINDING(FAmbientOcclusionParameters, 0x1394c0);
	// This seems to not be in LEC as a struct but the patterns are there. LE1/2 has this in its own function.
	HOOK_STRUCT_BINDING3(FMaterialBaseShaderParameters, 0x28d910);
	HOOK_STRUCT_BINDING(FFogVolumeVertexShaderParameters, 0x2242c0); // LEC labels this as FConstantDensityPolicy_VertexShaderParametersType
	HOOK_STRUCT_BINDING(FHeightFogVertexShaderParameters, 0x2243c0);
	HOOK_STRUCT_BINDING(FHBAOShaderParameters, 0x139540);
	//HOOK_STRUCT_BINDING(FDirectionalLightPolicyPixelParameters, );
	HOOK_STRUCT_BINDING(FGammaShaderParameters, 0x2579c0);
	HOOK_STRUCT_BINDING(FColorRemapShaderParameters, 0x2579c0);
	// LE2
	HOOK_STRUCT_BINDING(FMaterialVertexShaderParameters, 0x28d9c0);

	// Direct addresses - Functions
	GetShaderTypeList = (tGetShaderTypeList)SDKInitializer::Instance()->GetAbsoluteAddress(0x35a1a0); // LE2
	GetVertexFactoryTypeList = (tGetVertexFactoryTypeList)SDKInitializer::Instance()->GetAbsoluteAddress(0x8dfcf0); // LE2
	CreateArchiveProxy = (tCreateArchiveProxy)SDKInitializer::Instance()->GetAbsoluteAddress(0x21d6d0); // LE2
	_CreateName = (tSFXNameConstructor)SDKInitializer::Instance()->GetAbsoluteAddress(0x78640); // LE2

	// Direct addresses - VTables
	MemoryArchiveReaderVTableStart = SDKInitializer::Instance()->GetAbsoluteAddress(0xfbe178); // LE2 - FMemoryReader_VTable
	MemoryArchiveWriterVTableStart = SDKInitializer::Instance()->GetAbsoluteAddress(0xfbe080); // LE2 - FMemoryWriter_VTable
	FileReaderVTableStart = SDKInitializer::Instance()->GetAbsoluteAddress(0xfb9fe8); // LE2 - FArchiveFileReader_VTable
	StringArchiveProxyVTableStart = SDKInitializer::Instance()->GetAbsoluteAddress(0x101e1c0); // LE2 - FGlobalShaderCacheArchive_VTable

	// Direct addresses - Variables
	GFileManager = (FFileManager**)SDKInitializer::Instance()->GetAbsoluteAddress(0x16823c0); // LE2
	GError = (FArchive**)SDKInitializer::Instance()->GetAbsoluteAddress(0x1682348); // LE2

	AllHooksInstalled = true;
	return true;


	// OLD CODE ===================
	// Not used but left as an exercise to the reader.

	// Get a vertex factory as it seems necessary...
	auto vertexFactoryTypeListPointer = GetVertexFactoryTypeList();
	auto shaderTypeListPointer = GetShaderTypeList();
	std::this_thread::sleep_for(1500ms);
	auto vertexFactoryTypeList = *vertexFactoryTypeListPointer;

	auto shaderTypeList = *shaderTypeListPointer;
	std::cout << "LE2 Shader Types\n";
	while (shaderTypeList != nullptr) {
		auto serializableShader = shaderTypeList->CurrentItem->ConstructSerializedRef(shaderTypeList->CurrentItem);

		// Get the address of the Serialize function.
		auto vtable = (char*)GetVTable((void**)serializableShader);
		auto serialize = (void**)(vtable + 0x40);
		auto deref = *serialize; // Some shaders don't have parameters, so they just serialize like normal

		if (deref == FShaderSerialize) {
			// No parameters on this shader type.
			std::wcout << shaderTypeList->CurrentItem->Name << L" [No Parameters]" << std::endl;
		}
		else
		{
			TArray<BYTE> BaseData;
			DWORD zero = 0;
			FMemoryWriter ArW(BaseData);
			SetVTable((void**)&ArW, MemoryArchiveWriterVTableStart);

			// We have to serialize names as strings, otherwise lookup seems to fail. 
			// Wrap writer in names-only serializer
			FArchiveProxy ProxyArchive;
			CreateArchiveProxy((FArchiveProxy*)&ProxyArchive, &ArW);
			SetVTable((void**)&ProxyArchive, StringArchiveProxyVTableStart);

			// Write a fake FShader header block.
			BYTE platform = 5;
			BYTE frequency = shaderTypeList->CurrentItem->Frequency;

			ProxyArchive.VTable->Serialize(&ProxyArchive, &platform, 1);
			ProxyArchive.VTable->Serialize(&ProxyArchive, &frequency, 1);

			// Shader File Size
			DWORD four = 4;
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, four, 60000);
			// Shader bytes (dummy)
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, zero, 60000);
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, zero, 60000); // Parameter Map CRC

			// Guid
			DWORD one = 1;
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, one, 60000);
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, one, 60000);
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, one, 60000);
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, one, 60000);

			FName shaderName;
			CreateName(&shaderName, shaderTypeList->CurrentItem->Name);
			ProxyArchive.VTable->SerializeName(&ProxyArchive, shaderName);

			// Num Instructions
			ProxyArchive.VTable->SerializeInt(&ProxyArchive, zero, 60000);

			// Shader Parameters follow.
			if (shaderTypeList->CurrentItem->Frequency == SF_VERTEX) {
				// It will have a vertex factory parameter
				FName vfName;
				CreateName(&vfName, vertexFactoryTypeList->CurrentItem->Name);
				ProxyArchive.VTable->SerializeName(&ProxyArchive, vfName);
			}

			// Write a bunch of data
			for (int i = 400; i > 0; i--) {
				ProxyArchive.VTable->SerializeInt(&ProxyArchive, zero, sizeof(DWORD));
			}

			// DEBUG - Write out Data
			//auto writer = (*GFileManager)->CreateFileWriter(L"C:\\users\\mgame\\fakeshader.bin", 0, GError, 10000000);
			//writer->VTable->Serialize(writer, BaseData.Data, BaseData.Count);
			//writer->VTable->Destructor(writer); // flushes


			FMemoryReader ArR(BaseData);
			SetVTable((void**)&ArR, MemoryArchiveReaderVTableStart);

			// We have to serialize names as strings, otherwise lookup seems to fail. 
			// Wrap reader in names-only serializer
			FArchiveProxy ProxyArchiveR;
			CreateArchiveProxy((FArchiveProxy*)&ProxyArchiveR, &ArR);
			SetVTable((void**)&ProxyArchiveR, StringArchiveProxyVTableStart);

			std::wcout << shaderTypeList->CurrentItem->Name << L" " << deref << std::endl;
			serializableShader->Serialize(ProxyArchiveR);

		}


		shaderTypeList = shaderTypeList->NextItem;
	}

	return true;
}

