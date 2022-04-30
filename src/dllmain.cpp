#include "stdafx.h"
#include "./external/inih/INIReader.h"
#include "./external/SignatureScanner/SigScanner/SignatureScanner.h"
#include "./helper.hpp"
#include <stdio.h>
#include <iostream>

bool Proxy_Attach();
void Proxy_Detach();

using namespace std;

HMODULE baseModule = GetModuleHandle(NULL);

// Ini variables
bool bRemoveResolutionLimit;
bool bFixUI;
bool bCrashFix;
int iFPSCap;
bool bFPSCap;
bool bMovieFix;
int iShadowQuality;
bool bShadowQuality;
bool bColourFilter;
bool bFOVAdjust;
float fFOVAdjust;

// Variables
string procName = "re5dx9.exe";
SignatureScanner scanner = SignatureScanner((intptr_t)baseModule);
float fDesktopRight;
float fDesktopBottom;
float fDesktopAspect;
float fNativeAspect = 1.777777791f;

DWORD FPSCapReturnJMP;
float FPSCapValue;
void __declspec(naked) FPSCap_CC()
{
	__asm
	{
		divss xmm1, [FPSCapValue]
		movaps xmm0, xmm1
		jmp [FPSCapReturnJMP]
	}
}

DWORD MovieFixReturnJMP;
float MovieFixValue1;
float MovieFixValue2;
void __declspec(naked) MovieFix_CC()
{
	__asm
	{
		subss xmm1, xmm2
		addss xmm0, xmm2
		movss xmm0, [MovieFixValue1]
		movss xmm3, [MovieFixValue2]
		jmp [MovieFixReturnJMP]
	}
}

DWORD ShadowQualityReturnJMP;
int ShadowQualityValue;
void __declspec(naked) ShadowQuality_CC()
{
	__asm
	{
		mov eax, [ShadowQualityValue]
		add eax, 15
		push esi
		mov esi, ecx
		and eax, -16
		jmp [ShadowQualityReturnJMP]
	}
}

DWORD FOVNormalReturnJMP;
void __declspec(naked) FOVNormal_CC()
{
	__asm
	{
		fld dword ptr[eax + 0x24]
		fadd [fFOVAdjust]
		mov eax, [esi + 0x2AC]
		jmp [FOVNormalReturnJMP]
	}
}

DWORD FOVAimingReturnJMP;
void __declspec(naked) FOVAiming_CC()
{
	__asm
	{
		fld dword ptr[eax + 0x24]
		fadd [fFOVAdjust]
		mov eax, [esi + 0x2B8]
		jmp [FOVAimingReturnJMP]
	}
}

void ReadConfig()
{
	INIReader config("RE5Fix.ini");

	bFixUI = config.GetBoolean("Fix UI Scaling", "Enabled", true);
	bCrashFix = config.GetBoolean("Fix Ultrawide Crash", "Enabled", true);
	bRemoveResolutionLimit = config.GetBoolean("Remove Resolution Limits", "Enabled", true);
	iFPSCap = config.GetInteger("FPS Cap", "Value", -1);
	bFPSCap = config.GetInteger("FPS Cap", "Enabled", true);
	bMovieFix = config.GetBoolean("Fix Movies", "Enabled", true);
	iShadowQuality = config.GetInteger("Shadow Quality", "Value", -1);
	bShadowQuality = config.GetBoolean("Shadow Quality", "Enabled", true);
	bColourFilter = config.GetBoolean("Remove Colour Filter", "Enabled", true);
	bFOVAdjust = config.GetBoolean("Increase FOV", "Enabled", true);
	fFOVAdjust = config.GetFloat("Increase FOV", "Value", -1);

	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);
	fDesktopRight = (float)desktop.right;
	fDesktopBottom = (float)desktop.bottom;
	fDesktopAspect = fDesktopRight / fDesktopBottom;
}

void UIFix()
{
	if (bFixUI && fDesktopAspect > 1.8f)
	{
		// re5dx9.exe+1F43DF - 8B 83 E04E0000 - mov eax,[ebx+00004EE0]
		Memory::PatchBytes((intptr_t)baseModule + 0x1F43DF, "\xB8\x02\x00\x00\x00\x90", 6);
		// re5dx9.exe+1F2D06 - 8B 81 E04E0000 - mov eax, [ecx + 00004EE0]
		Memory::PatchBytes((intptr_t)baseModule + 0x1F2D06, "\xB8\x02\x00\x00\x00\x90", 6);
		#if _DEBUG
		std::cout << "UI scaling mode set to: " << "2" << std::endl;
		#endif	
	}
}


void ResolutionLimits()
{
	if (bRemoveResolutionLimit)
	{
		//re5dx9.exe + 387B7E - 8B 0D 60982301 - mov ecx, [re5dx9.exe + E39860]

		// Address of signature = re5dx9.exe + 0x00387B7E
		//"\x8B\x0D\x00\x00\x00\x00\x8A\x41", "xx????xx"
		//	"8B 0D ? ? ? ? 8A 41"

		intptr_t resLimitScanResult = (scanner.scan("8B 0D ? ? ? ? 8A 41") + 2);
		DWORD resLimitAddress = *(DWORD*)*(DWORD*)resLimitScanResult;
		int resLimitValue1 = *(int*)(resLimitAddress + 0x50) = (int)131072;
		int resLimitValue2 = *(int*)(resLimitAddress + 0x54) = (int)131072;
		#if _DEBUG
		std::cout << "Resolution limit = " << resLimitValue1 << "x" << resLimitValue2 << std::endl;
		#endif	
	}
}

void UncapFPS()
{
	if (bFPSCap && iFPSCap == 0) // Don't leave it at 0, assume they want it "uncapped".
	{
		iFPSCap = 999;
	}

	if (bFPSCap && iFPSCap > 120)
	{
		// re5dx9.exe+4B45C - F3 0F5E 4E 3C         - divss xmm1,[esi+3C] -> (30/120) || 120 FPS cap
		int FPSCapHookLength = 8;
		DWORD FPSCapAddress = (intptr_t)baseModule + 0x4B45C;
		FPSCapValue = (float)iFPSCap;
		FPSCapReturnJMP = FPSCapAddress + FPSCapHookLength;
		Memory::Hook((void*)FPSCapAddress, FPSCap_CC, FPSCapHookLength);
		#if _DEBUG
		std::cout << "FPS Cap set to: " << (int)iFPSCap << std::endl;
		#endif	
	}
}


void CrashFix()
{
	if (bCrashFix && fDesktopAspect > 1.8f)
	{
		// re5dx9.exe+CBEA24 -> 9/16 = 0.5625
		float newAR = fDesktopBottom / fDesktopRight; // Backwards, just the way it should be.
		Memory::Write((intptr_t)baseModule + 0xCBEA24, newAR);
		#if _DEBUG
		std::cout << "Aspect ratio crash fix enabled. Cutscene AR set to: " << newAR << std::endl;
		#endif	
	}
}

void MovieFix()
{
	if (bMovieFix && fDesktopAspect > 1.8f)
	{
		//re5dx9.exe + 255700 - F3 0F5C CA - subss xmm1, xmm2
		int MovieFixHookLength = 8;
		DWORD MovieFixAddress = (intptr_t)baseModule + 0x255700;
		MovieFixValue1 = (float)fNativeAspect / fDesktopAspect;
		MovieFixValue2 = -MovieFixValue1;
		MovieFixReturnJMP = MovieFixAddress + MovieFixHookLength;
		Memory::Hook((void*)MovieFixAddress, MovieFix_CC, MovieFixHookLength);
		#if _DEBUG
		std::cout << "Pre-rendered movie playback fix enabled." << std::endl;
		#endif
	}
}

void IncreaseQuality()
{
	if (bShadowQuality && iShadowQuality >= 1024)
	{
		// re5dx9.exe+252805 - 83 C0 0F              - add eax,0F
		// Shadow quality. Low=256, Med=512, High=1024.
		int ShadowQualityHookLength = 9;
		DWORD ShadowQualityAddress = (intptr_t)baseModule + 0x252805;
		ShadowQualityValue = iShadowQuality;
		ShadowQualityReturnJMP = ShadowQualityAddress + ShadowQualityHookLength;
		Memory::Hook((void*)ShadowQualityAddress, ShadowQuality_CC, ShadowQualityHookLength);
		#if _DEBUG
		std::cout << "Shadow quality set to " << iShadowQuality << std::endl;
		#endif
	}
}

void ColourFilter()
{
	if (bColourFilter)
	{
		// re5dx9.exe+945D8 - 0F87 87660000 - ja re5dx9.exe+9AC65 || Jump to this instead.
		Memory::PatchBytes((intptr_t)baseModule + 0x945D8, "\xE9\x88\x66\x00\x00\x90", 6);
		#if _DEBUG
		std::cout << "Colour filter disabled." << std::endl;
		#endif
	}
}

void FOVAdjust()
{
	if (bFOVAdjust && fFOVAdjust > 0)
	{
		// Normal FOV = 45
		// re5dx9.exe + 44AB92 - D9 40 24 - fld dword ptr[eax + 24]
		int FOVNormalHookLength = 9;
		DWORD FOVNormalAddress = (intptr_t)baseModule + 0x44AB92;
		FOVNormalReturnJMP = FOVNormalAddress + FOVNormalHookLength;
		Memory::Hook((void*)FOVNormalAddress, FOVNormal_CC, FOVNormalHookLength);

		// Aiming FOV = 33
		// re5dx9.exe + 44A9DE - D9 40 24 - fld dword ptr[eax + 24]
		int FOVAimingHookLength = 9;
		DWORD FOVAimingAddress = (intptr_t)baseModule + 0x44A9DE;
		FOVAimingReturnJMP = FOVAimingAddress + FOVAimingHookLength;
		Memory::Hook((void*)FOVAimingAddress, FOVAiming_CC, FOVAimingHookLength);

		// No Weapon FOV = 42
		//

		#if _DEBUG
		std::cout << "FOV increased by: " << fFOVAdjust << std::endl;
		#endif
	}
}


DWORD __stdcall Main(void*)
{
	Sleep(1000); // delay first
	
	#if _DEBUG
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	std::cout << "Console initiated" << std::endl;
	#endif	
	ReadConfig();
	UIFix();
	ResolutionLimits();
	UncapFPS();
	CrashFix();
	MovieFix();
	IncreaseQuality();
	ColourFilter();
	FOVAdjust();

	return true; // end thread
}

HMODULE ourModule = 0;

void Patch_Uninit()
{

}

BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ourModule = hModule;
		Proxy_Attach();

		CreateThread(NULL, 0, Main, 0, NULL, 0);
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Patch_Uninit();

		Proxy_Detach();
	}

	return TRUE;
}

