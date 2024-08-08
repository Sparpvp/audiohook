#include <iostream>
#include <Windows.h>

#include "opcodescanner.h"

#include "beaengine-5.3.0/headers/BeaEngine.h"
#include <capstone/capstone.h>

template<typename T>
T GetDisplacement(cs_insn* inst, uint8_t offset)
{
	T disp;
	memcpy(&disp, &inst->bytes[offset], sizeof(T));
	return disp;
}

// Relocate RIP-Relative addresses so that they point to the proper location
void RelocateAddresses(cs_insn* inst, void* dstLocation)
{
	cs_x86* x86 = &(inst->detail->x86);
	uint8_t offset = x86->encoding.disp_offset;

	uint64_t displacement = inst->bytes[x86->encoding.disp_offset];
	switch (x86->encoding.disp_size)
	{
	case 1:
		{
			int8_t disp = GetDisplacement<uint8_t>(inst, offset);
			disp -= uint64_t(dstLocation) - inst->address;
			memcpy(&inst->bytes[offset], &disp, 1);
		}
	break;
	case 2:
		{
			int16_t disp = GetDisplacement<uint16_t>(inst, offset);
			disp -= uint64_t(dstLocation) - inst->address;
			memcpy(&inst->bytes[offset], &disp, 2);
		}
	break;
	case 4:
		{
			int32_t disp = GetDisplacement<int32_t>(inst, offset);
			disp -= uint64_t(dstLocation) - inst->address;
			memcpy(&inst->bytes[offset], &disp, 4);
		}
		break;
	default:
		std::cerr << "Length not supported: " << x86->encoding.disp_size << std::endl;
	}
}

// Call this in a loop to check if relocation is needed on the stolen bytes
bool IsRIPRelativeInstr(const cs_insn* inst) {
	if (inst->id == 0) {
		return false;
	}

	// Check each operand of the instruction
	for (int i = 0; i < inst->detail->x86.op_count; i++) {
		cs_x86_op* op = &(inst->detail->x86.operands[i]);

		// Check if the operand is a memory operand and uses RIP-relative addressing
		if (op->type == X86_OP_MEM && op->mem.base == X86_REG_RIP) {
			return true;
		}
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

	while ((disasm.EIP - (UIntPtr)hkFunc) < maxSize || maxSize == -1)
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