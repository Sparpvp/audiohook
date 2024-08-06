#pragma once

#include <iostream>
#include <Windows.h>
#include <memory>

#include "hooker.h"
#include "registersaver.h"

using std::unique_ptr;

unique_ptr<VTEntrySwapper> Trampoline64(BYTE *hkFunc, BYTE **origFunc, void (*__register_saver)(), DWORD offset);

LPVOID ConstructGateway(_In_ uintptr_t startHkFunc, _In_ void (*__register_saver)(), _In_opt_ DWORD offset, _In_ bool jmpBack);
void Detour(uintptr_t ptrSrc, uintptr_t hookFunc, uintptr_t ptrTramp);
uintptr_t BuildUnknownGateway(uintptr_t beginHook, int &minBytes);
uintptr_t ResolveJumpAddress(BYTE *funcPtr);

// Function happily copied from Kyle Halladay. Shoutout to his blog.
void *AllocatePageNearAddress(void *targetAddr);

// void DetourHookedFunction(uintptr_t startHkFunc, LPVOID gatewayAddress);
// void PatchEpilogue(uintptr_t startHkFunc, LPVOID gatewayAddress, LPVOID origFunc);