#pragma once

// Setup of this object must be done before using it. You should not call these directly!
class UnrealMalloc
{
public:
	static UnrealMalloc GMalloc;

private:
	// Changed to INT as DWORD requires typedef
	void* (*appMalloc)(int, int, int) = nullptr;
	void (*appFree)(void*, int) = nullptr;
	void* (*appRealloc)(void*, int, int, int) = nullptr;

public:
	// These must be set by a project external to LE3-SDK as it does not have SPI access.
	bool (*CreateMalloc)(void**) = nullptr;
	bool (*CreateFree)(void**) = nullptr;
	bool (*CreateRealloc)(void**) = nullptr;
	void* Malloc(int count, int alignment = 0x10);
	void Free(void* allocation);
	void* Realloc(void* original, int count, int alignment = 0x10);
};