#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "includes\injector\injector.hpp"
#include <cstdint>

float LeftGroupX, LeftGroupY, RightGroupX, RightGroupY;

DWORD dword_E5FBC0 = 0xE5FBC0;
DWORD dword_E5FBC4 = 0xE5FBC4;
float ScreenAspectRatio;
float ratio_05625 = 0.5625f;
float MiniMapXPos = -320.0f;
float MiniMapXPosSubtraction = -320.0f;
float MiniMapXPosAddition;
float MiniMapXPosOffset = -7.0f;
DWORD ForceWidescreenHUDCodeCaveExit = 0x520CA3;

void(__cdecl* FE_Object_SetCenter)(DWORD* FEObject, float _PositionX, float _PositionY) = (void(__cdecl*)(DWORD*, float, float))0x57CFE0;
void(__cdecl* FE_Object_GetCenter)(DWORD* FEObject, float* PositionX, float* PositionY) = (void(__cdecl*)(DWORD*, float*, float*))0x5958E0;
void* (__cdecl* FEObject_FindObject)(const char* pkg_name, unsigned int obj_hash) = (void* (__cdecl*)(const char*, unsigned int))0x5859C0;

int __stdcall cFEng_QueuePackageMessage_Hook(unsigned int MessageHash, char const* FEPackageName, DWORD* FEObject)
{
	int ResX, ResY;
	float Difference;

	ResX = *(int*)0xE5FBC0;
	ResY = *(int*)0xE5FBC4;

	if (ResX != 0 && ResY != 0) Difference = (((float)ResX / (float)ResY) * 360) - 480; // Calculate position difference for current aspect ratio
	else Difference = 0; // 4:3 if we can't get any values

	/*injector::WriteMemory<float*>(0x5D52FB, &Difference, true);
	injector::WriteMemory<float*>(0x5D5358, &Difference, true);*/

	DWORD* LeftGroup = (DWORD*)FEObject_FindObject(FEPackageName, 0x1603009E); // "HUD_SingleRace.fng", leftgrouphash

	if (LeftGroup) // Move left group
	{
		FE_Object_GetCenter(LeftGroup, &LeftGroupX, &LeftGroupY);
		FE_Object_SetCenter(LeftGroup, LeftGroupX - Difference, LeftGroupY);
	}

	DWORD* RightGroup = (DWORD*)FEObject_FindObject(FEPackageName, 0x5D0101F1); // "HUD_SingleRace.fng", rightgrouphash

	if (RightGroup) // Move right group
	{
		FE_Object_GetCenter(RightGroup, &RightGroupX, &RightGroupY);
		FE_Object_SetCenter(RightGroup, RightGroupX + Difference, RightGroupY);
	}

	return 1;
}

void __declspec() MiniMapXPosCodeCave1()
{
	__asm 
	{
		

		movss xmm2, dword ptr ds : [MiniMapXPos] // -320
		mulss xmm2, dword ptr ds : [ScreenAspectRatio] // Multiplies by screen ratio
		movss dword ptr ds : [MiniMapXPosAddition] , xmm2 // Stores ratio Result
		subss xmm2, dword ptr ds : [MiniMapXPosSubtraction] // -320
		addss xmm2, dword ptr ds : [MiniMapXPosAddition] // Adds ratio result
		addss xmm2, dword ptr ds : [MiniMapXPosOffset] // Controls position
		ret
	}
}

void __declspec() MiniMapXPosCodeCave2()
{

	__asm 
	{
		movss xmm0, dword ptr ds : [MiniMapXPos] // -320
		mulss xmm0, dword ptr ds : [ScreenAspectRatio] // Multiplies by screen ratio
		movss dword ptr ds : [MiniMapXPosAddition] , xmm0 // Stores ratio Result
		subss xmm0, dword ptr ds : [MiniMapXPosSubtraction] // -320
		addss xmm0, dword ptr ds : [MiniMapXPosAddition] // Adds ratio result
		addss xmm0, dword ptr ds : [MiniMapXPosOffset] // Controls position
		ret
	}
}


void __declspec(naked) MiniMapXPosCodeCave3()
{
	__asm 
	{
		push eax
		mov eax, [dword_E5FBC0]
		fild dword ptr ds : [eax] // Res X
		mov eax, [dword_E5FBC4]
		fidiv dword ptr ds : [eax] // Res Y 
		fmul dword ptr ds : [ratio_05625] // 0.5625
		fstp dword ptr ds : [ScreenAspectRatio]
		pop eax

		movss xmm0, dword ptr ds : [MiniMapXPos] // -320
		mulss xmm0, dword ptr ds : [ScreenAspectRatio] // Multiplies by screen ratio
		movss dword ptr ds : [MiniMapXPosAddition] , xmm0 // Stores ratio Result
		subss xmm0, dword ptr ds : [MiniMapXPosSubtraction] // -320
		addss xmm0, dword ptr ds : [MiniMapXPosAddition] // Adds ratio result
		addss xmm0, dword ptr ds : [MiniMapXPosOffset] // Controls position
		movss dword ptr ds : [edx + 0x1C] , xmm0
		ret
	}
}

void __declspec(naked) ForceWidescreenHUDCodeCave()
{
	__asm {
		mov eax, 0x01
		mov dword ptr ds : [ecx] , eax
		pop esi
		mov al, bl
		jmp ForceWidescreenHUDCodeCaveExit
	}
}

void Init()
{
	// HUD Adapter
	injector::MakeCALL(0x5C3E65, cFEng_QueuePackageMessage_Hook, true);
	injector::MakeCALL(0x5C3E88, cFEng_QueuePackageMessage_Hook, true);

	// MiniMap Position
	injector::MakeCALL(0x5C7965, MiniMapXPosCodeCave1, true);
	injector::MakeNOP(0x5C796A, 3, true);
	injector::MakeCALL(0x5C7C65, MiniMapXPosCodeCave1, true);
	injector::MakeNOP(0x5C7C6A, 3, true);
	injector::MakeCALL(0x5CC815, MiniMapXPosCodeCave1, true);
	injector::MakeNOP(0x5CC81A, 3, true);
	injector::MakeCALL(0x5C1E1F, MiniMapXPosCodeCave2, true);
	injector::MakeNOP(0x5C1E24, 3, true);
	injector::MakeCALL(0x5C3EBD, MiniMapXPosCodeCave3, true);

	// Forces Widescreen HUD
	injector::MakeJMP(0x520C9E, ForceWidescreenHUDCodeCave, true);
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Init();
	}
	return TRUE;
}