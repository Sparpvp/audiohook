#pragma once

#include <Windows.h>
#include "beaengine-5.3.0/headers/BeaEngine.h"
#include <capstone/capstone.h>
#include "templatefuncs.h"

DWORD FindPrologue(const BYTE* hkFunc, const int maxSize);
DWORD GetEndOfHook(const BYTE* hkFunc, const int maxSize, LPVOID g_oFn);
DWORD IsStartOfEpilogue(const BYTE* hkFunc, const int maxSize);
void RelocateAddresses(cs_insn* inst, void* dstLocation);
bool IsRIPRelativeInstr(const cs_insn* inst);

bool instructionIsLea(DISASM& disasm);
bool instructionUsesRsp(DISASM& disasm);
