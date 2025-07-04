#include <Windows.h>
#include <tlhelp32.h>
#include <cstdio>

#pragma once

template<typename T> struct TArray;
struct FNameEntry;
class UObject;


#define LEx_MODULE_NAME		L"MassEffect2.exe"
#define LEx_NAME_POOLS		0x1668A10       // RVA of the name pools
#define LEx_OBJOBJECTS		0x173CC48       // RVA of the UObject::ObjObjects


class SDKInitializer
{
private:
	BYTE* ModuleBase;

	inline BYTE* GetModuleBaseAddress(wchar_t* moduleName)
	{
		auto pid = GetCurrentProcessId();

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		BYTE* baseAddress = nullptr;

		if (INVALID_HANDLE_VALUE != snapshot)
		{
			MODULEENTRY32 moduleEntry = { 0 };
			moduleEntry.dwSize = sizeof(MODULEENTRY32);

			if (Module32First(snapshot, &moduleEntry))
			{
				do
				{
					if (0 == wcscmp(moduleEntry.szModule, moduleName))
					{
						baseAddress = moduleEntry.modBaseAddr;
						break;
					}
				} while (Module32Next(snapshot, &moduleEntry));
			}
			CloseHandle(snapshot);
		}

		return baseAddress;
	}

	SDKInitializer()
	{
		ModuleBase = GetModuleBaseAddress(LEx_MODULE_NAME);
	}

public:

	inline FNameEntry** GetBioNamePools() const noexcept
	{
		return &*(FNameEntry**)(ModuleBase + LEx_NAME_POOLS);
	}
	inline struct TArray<class UObject*>* GetObjects() const noexcept
	{
		return &*(struct TArray<class UObject*>*)(ModuleBase + LEx_OBJOBJECTS);
	}

	inline static SDKInitializer* Instance()
	{
		static SDKInitializer* initializer = nullptr;
		if (!initializer)
		{
			initializer = new SDKInitializer{};
		}
		return initializer;
	}

	inline uintptr_t GetModuleBaseRelativeAddress(const void* absoluteAddress) const noexcept
	{
		return static_cast<const BYTE*>(absoluteAddress) - ModuleBase;
	}

	inline void* GetAbsoluteAddress(const uintptr_t moduleBaseRelativeAddress) const noexcept
	{
		return ModuleBase + moduleBaseRelativeAddress;
	}
};
