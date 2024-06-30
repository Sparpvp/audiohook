#include <iostream>
#include <Windows.h>

#include "opcodescanner.h"

#include "beaengine-5.3.0/headers/BeaEngine.h"

// This is incredibly sketchy, todo use opcodes / something else
bool instructionIsLea(DISASM& disasm)
{
	// std::cout << "istr: " << disasm.CompleteInstr << std::endl;
	if (std::string(disasm.CompleteInstr).find("lea") != std::string::npos)
		return true;

	return false;
}

DWORD FindPrologue(const BYTE* hkFunc, const int maxSize)
{
	DISASM disasm;
	memset(&disasm, 0, sizeof(DISASM));
	disasm.EIP = (UIntPtr)hkFunc;
	disasm.Archi = 64;

	bool isLea = false;
	while ((disasm.EIP - (UIntPtr)hkFunc) < maxSize && !isLea)
	{ 
		int len = Disasm(&disasm);

		isLea = instructionIsLea(disasm);
		if (isLea)
			return (disasm.EIP - (UIntPtr)hkFunc + len);

		disasm.EIP += len;
	}
}

bool instructionUsesRsp(DISASM& disasm)
{
	/*if (
		disasm.Operand1.Registers.gpr & REG4 ||
		disasm.Operand2.Registers.gpr & REG4 ||
		disasm.Operand1.Memory.BaseRegister & REG4 ||
		disasm.Operand2.Memory.BaseRegister & REG4 ||
		disasm.Instruction.ImplicitUsedRegs.gpr & REG4 ||
		disasm.Instruction.ImplicitModifiedRegs.gpr & REG4
		)
	{
		return true;
	}*/
	if (std::string(disasm.CompleteInstr).find("rsp") != std::string::npos)
	{
		std::cout << "contiene rsp\n";
		return true;
	}

	return false;
}

DWORD GetEndOfHook(const BYTE* hkFunc, const int maxSize, LPVOID g_oFn)
{
	DISASM disasm;
	memset(&disasm, 0, sizeof(DISASM));
	disasm.EIP = (UIntPtr)hkFunc;
	disasm.Archi = 64;

	while ((disasm.EIP - (UIntPtr)hkFunc) < maxSize || maxSize == -1)
	{
		int len = Disasm(&disasm);

		if (disasm.Instruction.AddrValue == (uintptr_t)g_oFn)
			return (disasm.EIP - (UIntPtr)hkFunc);

		disasm.EIP += len;
	}
}

DWORD IsStartOfEpilogue(const BYTE* hkFunc, const int maxSize)
{
	DISASM disasm;
	memset(&disasm, 0, sizeof(DISASM));
	disasm.EIP = (UIntPtr)hkFunc;
	disasm.Archi = 64;

	while((disasm.EIP - (UIntPtr)hkFunc) < maxSize || maxSize == -1)
	{
		int len = Disasm(&disasm);

		std::cout << "Instruction: " << std::hex << disasm.Instruction.Opcode << std::endl;
		if (instructionUsesRsp(disasm))
		{
			std::cout << "FOUND RSP USAGE!" << std::endl;
			return (disasm.EIP - (UIntPtr)hkFunc);
		}

		disasm.EIP += len;
	}
}