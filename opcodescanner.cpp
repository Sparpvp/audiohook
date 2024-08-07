#include <iostream>
#include <Windows.h>

#include "opcodescanner.h"

#include "beaengine-5.3.0/headers/BeaEngine.h"

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

void RelocateAddresses(LPVOID src, int len)
{

}

// Returns false if relocation is not needed, otherwise returns true
// Call this in a loop to check for relocation on the stolen bytes
bool IsRelativeAddressing(DISASM instr)
{
	if (instr.Operand1.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand2.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand3.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand4.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand5.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand6.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand7.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand8.OpType == MEMORY_TYPE + RELATIVE_ ||
		instr.Operand9.OpType == MEMORY_TYPE + RELATIVE_)
	{
		return true;
	}

	return false;
}

// This is incredibly sketchy, todo use opcodes / something else
bool instructionIsLea(DISASM& disasm)
{
	// std::cout << "istr: " << disasm.CompleteInstr << std::endl;
	if (std::string(disasm.CompleteInstr).find("lea") != std::string::npos)
		return true;

	return false;
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