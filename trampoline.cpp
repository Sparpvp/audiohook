#include <memory>

#include "trampoline.h"
#include "opcodescanner.h"
#include "templatefuncs.h"
#include "ptrglobal.hpp"

using std::unique_ptr;

uintptr_t ResolveJumpAddress(BYTE* funcPtr)
{
	BYTE* instruction = (BYTE*)funcPtr;

	if (*instruction != 0xE9)
	{
		std::cout << "ERR: No jump detected." << std::endl;
		return (uintptr_t)nullptr;
	}

	DWORD offset = *(DWORD*)(instruction + 1);
	uintptr_t jumpAddress = (uintptr_t)funcPtr + offset + 5;

	return jumpAddress;
}

unique_ptr<VTEntrySwapper> Trampoline64(BYTE* hkFunc, BYTE** origFunc)
{
	// Calculate relative address to jump back
	uintptr_t hookJumpAddress = ResolveJumpAddress((BYTE*)hkFunc);

	DWORD offsetToEndHook = GetEndOfHook((BYTE*)hookJumpAddress, -1, origFunc);
	uintptr_t hookEndJumpAddress = hookJumpAddress + offsetToEndHook;

	// Build __pusha_64 and __popa_64 gateways
	LPVOID pushaGateway = ConstructGateway(
		hookJumpAddress,
		__pusha_64,
		NULL
	);
	LPVOID popaGateway = ConstructGateway(
		hookEndJumpAddress,
		__popa_64,
		*origFunc
	);

	// Modify hkFunc to jump to the first gateway and adjust the prologue
	DetourHookedFunction(hookJumpAddress, pushaGateway);

	PatchEpilogue(hookEndJumpAddress, popaGateway, (LPVOID)origFunc);

	return std::make_unique<VTEntrySwapper>(VTEntrySwapper());
	}

void PatchEpilogue(uintptr_t startHkFunc, LPVOID gatewayAddress, LPVOID origFunc)
{
	DWORD offsetToEndHook = GetEndOfHook(
		(BYTE*)startHkFunc,
		1000,
		origFunc
	);

	constexpr int sizeof86relJmp = 5;
	DWORD prot;
	VirtualProtect(
		reinterpret_cast<LPVOID>(startHkFunc + offsetToEndHook), 
		sizeof86relJmp, 
		PAGE_EXECUTE_READWRITE, 
		&prot
	);
	DWORD relPopaAddress = (uintptr_t)gatewayAddress - startHkFunc - 5;
	*(BYTE*)(startHkFunc + offsetToEndHook) = 0xE9;
	*(DWORD*)((BYTE*)startHkFunc + offsetToEndHook + 1) = relPopaAddress;
	VirtualProtect(
		reinterpret_cast<LPVOID>(startHkFunc + offsetToEndHook),
		sizeof86relJmp, 
		prot, 
		&prot
	);
}

void DetourHookedFunction(uintptr_t startHkFunc, LPVOID gatewayAddress)
{
	// Find the end of the prologue, delimited by the first lea instruction.
	// This could be inaccurate, but for our needs it's good enough.
	DWORD offsetToEndPrologue = FindPrologue((BYTE*)startHkFunc, 200);
	// NOP the prologue at the start of the hookFunction and change its memory protection
	DWORD prot;
	VirtualProtect((LPVOID)startHkFunc, offsetToEndPrologue, PAGE_EXECUTE_READWRITE, &prot);
	memset((void*)startHkFunc, 0x90, offsetToEndPrologue);
	// Write the jump to the gate at the start of the hooked function
	uintptr_t offsetToGate = (uintptr_t)gatewayAddress - startHkFunc - 5;
	*(BYTE*)startHkFunc = 0xE9;
	*(DWORD*)((BYTE*)startHkFunc + 1) = offsetToGate;
	VirtualProtect((LPVOID)startHkFunc, offsetToEndPrologue, prot, &prot);

	//std::cout << "Offset to gate " << std::hex << offsetToGate << std::endl;
	//std::cout << "Addy: " << std::hex << (uintptr_t)gatewayAddress << std::endl;
	//std::cout << "pusha64: " << std::hex << __pusha_64 << std::endl;
	std::cout << "hkFunc: " << std::hex << (uintptr_t)startHkFunc << std::endl;
	//std::cout << "offset to end prologue: " << std::hex << offsetToEndPrologue << std::endl;
}

// Returns a pointer to the newly allocated gateway, near to the startHkFunc supplied.
/*
	origFunc: MAY be passed as NULL if we don't want to specify a particular jump back addy.
*/
LPVOID ConstructGateway(
	_In_ uintptr_t startHkFunc, 
	_In_ void (*__register_saver)(), 
	_In_opt_ BYTE* origFunc
)
{
	/*
		sub/add rsp, 100h = 7 bytes
		push rax...registers = 1 byte per instruction -> 7 bytes
		push r8...r15 = 2 bytes per instruction -> 8*2 bytes
	*/
	constexpr int pushaSize = 7 + 8 * 2 + 7;

	// Try to allocate the gateway near enough
	// that's because we want to avoid having 64bit address displacement 
	// (cause x64 jmps suck, lol)
	LPVOID gatewayAddress = AllocatePageNearAddress((void*)startHkFunc);
	if (gatewayAddress == NULL)
		return nullptr;

	// Get __pusha_64 | __popa_64 start address
	uintptr_t pushaJumpAddress = ResolveJumpAddress((BYTE*)__register_saver);
	// Copy its instructions into newly allocated gateway
	memcpy(gatewayAddress, (void*)pushaJumpAddress, pushaSize);
	// Calculate jump back offset
	uintptr_t relativeJmpBackAddress = NULL;
	if (origFunc == NULL)
		relativeJmpBackAddress = (BYTE*)(startHkFunc-pushaSize) - (BYTE*)gatewayAddress;
	else
		relativeJmpBackAddress = origFunc - gatewayAddress-pushaSize - 5;
	// Add the jump back; at the end of the gate
	*(BYTE*)((uintptr_t)gatewayAddress + pushaSize) = 0xE9;
	*(uintptr_t*)((uintptr_t)gatewayAddress + pushaSize + 1) = relativeJmpBackAddress;

	// std::cout << "Gate addr: " << gatewayAddress << std::endl;

	return gatewayAddress;
}

void* AllocatePageNearAddress(void* targetAddr)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	const uint64_t PAGE_SIZE = sysInfo.dwPageSize;

	uint64_t startAddr = (uint64_t(targetAddr) & ~(PAGE_SIZE - 1)); //round down to nearest page boundary
	uint64_t minAddr = min(startAddr - 0x7FFFFF00, (uint64_t)sysInfo.lpMinimumApplicationAddress);
	uint64_t maxAddr = max(startAddr + 0x7FFFFF00, (uint64_t)sysInfo.lpMaximumApplicationAddress);

	uint64_t startPage = (startAddr - (startAddr % PAGE_SIZE));

	uint64_t pageOffset = 1;
	while (1)
	{
		uint64_t byteOffset = pageOffset * PAGE_SIZE;
		uint64_t highAddr = startPage + byteOffset;
		uint64_t lowAddr = (startPage > byteOffset) ? startPage - byteOffset : 0;

		bool needsExit = highAddr > maxAddr && lowAddr < minAddr;

		if (highAddr < maxAddr)
		{
			void* outAddr = VirtualAlloc((void*)highAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (outAddr)
				return outAddr;
		}

		if (lowAddr > minAddr)
		{
			void* outAddr = VirtualAlloc((void*)lowAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (outAddr != nullptr)
				return outAddr;
		}

		pageOffset++;

		if (needsExit)
		{
			break;
		}
	}

	return nullptr;
}