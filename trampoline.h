#pragma once

#include <iostream>
#include <Windows.h>
#include <memory>

#include "hooker.h"
#include "registersaver.h"

using std::unique_ptr;

unique_ptr<VTEntrySwapper> Trampoline64(BYTE* hkFunc, BYTE** origFunc);

LPVOID ConstructGateway(_In_ uintptr_t startHkFunc, _In_ void (*__register_saver)(), _In_opt_ BYTE* origFunc);
void DetourHookedFunction(uintptr_t startHkFunc, LPVOID gatewayAddress);
void PatchEpilogue(uintptr_t startHkFunc, LPVOID gatewayAddress, LPVOID origFunc);

// Function happily copied from Kyle Halladay. Shoutout to his blog.
void* AllocatePageNearAddress(void* targetAddr);