#include "stdafx.h"
#include <stdio.h>
#include <iostream>

namespace Memory
{
	template<typename T>
	void Write(DWORD writeAddress, T value)
	{
		DWORD oldProtect;
		VirtualProtect((LPVOID)(writeAddress), sizeof(T), PAGE_EXECUTE_WRITECOPY, &oldProtect);
		*(reinterpret_cast<T*>(writeAddress)) = value;
		VirtualProtect((LPVOID)(writeAddress), sizeof(T), oldProtect, &oldProtect);
	}

	void PatchBytes(intptr_t address, const char* pattern, unsigned int numBytes)
	{
		DWORD oldProtect;
		VirtualProtect((LPVOID)address, numBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy((LPVOID)address, pattern, numBytes);
		VirtualProtect((LPVOID)address, numBytes, oldProtect, &oldProtect);
	}

	bool Hook(void* hookAddress, void* hookFunction, int hookLength) {
		if (hookLength < 5) {
			return false;
		}

		DWORD oldProtect;
		VirtualProtect(hookAddress, hookLength, PAGE_EXECUTE_WRITECOPY, &oldProtect);

		memset(hookAddress, 0x90, hookLength);

		DWORD relativeAddress = ((DWORD)hookFunction - (DWORD)hookAddress) - 5;

		*(BYTE*)hookAddress = 0xE9;
		*(DWORD*)((DWORD)hookAddress + 1) = relativeAddress;

		DWORD tmp;
		VirtualProtect(hookAddress, hookLength, oldProtect, &tmp);

		return true;
	}
}