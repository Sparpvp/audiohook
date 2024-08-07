#pragma once

#include <Windows.h>
#include "beaengine-5.3.0/headers/BeaEngine.h"
#include "templatefuncs.h"

DWORD FindPrologue(const BYTE* hkFunc, const int maxSize);
DWORD GetEndOfHook(const BYTE* hkFunc, const int maxSize, LPVOID g_oFn);
DWORD IsStartOfEpilogue(const BYTE* hkFunc, const int maxSize);
void RelocateAddresses(LPVOID src, int len);
bool IsRelativeAddressing(DISASM instr);

bool instructionIsLea(DISASM& disasm);
bool instructionUsesRsp(DISASM& disasm);
