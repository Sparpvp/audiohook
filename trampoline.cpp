#include <memory>

#include "trampoline.h"
#include "opcodescanner.h"
#include "templatefuncs.h"
#include "ptrglobal.hpp"

using std::unique_ptr;

unique_ptr<VTEntrySwapper> Trampoline64(
	BYTE *hkFunc,
	BYTE **origFunc,
	void (*__register_saver)(),
	DWORD offset
)
{
	// TODO: Should double check if these jump tables that need
	// to be resolved add some kind of compiler specificity.
	uintptr_t hookJumpAddress = ResolveJumpAddress(hkFunc);

	// Build __pusha_64 and __popa_64 gateways
	LPVOID regOpGateway = ConstructGateway(
		hookJumpAddress,
		__register_saver,
		0,
		true
	);

	/*
	BEGIN HOOK
	-> jmp detour
		detour:
			jmp pusha64 gatway
			|
			|	pusha64 gateway:
			|		__pusha_64
			|		jmp back
	(back:) |
			jmp trampoline
		trampoline:
			"stolenBytes" (occhio alla lunghezza degli opcode!)
			jmp back (next instruction)
	*/

	// Address of where some kind of regOp needs to be performed
	// Aka address of actual code, before or after the OrigFunc call
	uintptr_t beginHook = hookJumpAddress + offset;

	// Build the trampoline that executes the future stolen bytes
	int stolenLen = 0;
	uintptr_t stolenPtrTramp = BuildUnknownGateway(beginHook, stolenLen);
	DWORD oldProtect;
	VirtualProtect((LPVOID)beginHook, stolenLen, PAGE_EXECUTE_READWRITE, &oldProtect);
	memset((void*)beginHook, 0x90, stolenLen);
	VirtualProtect((LPVOID)beginHook, stolenLen, oldProtect, &oldProtect);

	// Jump to the gateway before jumping to the tramp
	Detour(beginHook, (uintptr_t)regOpGateway, stolenPtrTramp);

	return std::make_unique<VTEntrySwapper>(VTEntrySwapper());
}

void Detour(uintptr_t ptrSrc, uintptr_t hookFunc, uintptr_t ptrTramp)
{
	constexpr int pushaLen = 7 + 8 * 2 + 7;

	DWORD relHookFunc = hookFunc - ptrSrc - 5;

	DWORD oldProtect;
	VirtualProtect((LPVOID)ptrSrc, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	*(BYTE *)(ptrSrc) = 0xE9;
	*(DWORD *)(ptrSrc + 1) = relHookFunc;
	VirtualProtect((LPVOID)ptrSrc, 5, oldProtect, &oldProtect);

	// JMP to tramp after the gate
	DWORD relTrampAddr = (uintptr_t)(hookFunc + pushaLen) - ptrTramp - 5;

	*(BYTE *)(hookFunc + pushaLen) = 0xE9;
	*(DWORD *)(hookFunc + pushaLen + 1) = relTrampAddr;
}

uintptr_t BuildUnknownGateway(uintptr_t beginHook, int &minBytes)
{
	minBytes = 0;
	constexpr size_t jmpSizeBytes = 5;
	constexpr int pushaLen = 7 + 8 * 2 + 7;

	LPVOID gateAddr = AllocatePageNearAddress((void*)beginHook);

	// Get size of the bytes that are going to be stolen
	DISASM infos;
	memset(&infos, 0, sizeof(DISASM));
	infos.EIP = (UIntPtr)beginHook;
	infos.Archi = 64;
	while (minBytes < 2 * jmpSizeBytes)
	{
		int len = Disasm(&infos);
		infos.EIP += len;
		minBytes += len;
	}

	// Copy the aforementioned bytes in the gate
	memcpy((void*)gateAddr, (void*)beginHook, minBytes);

	// TODO FIX: problema strano, gateaddr sembra essere scritto correttamente
	// nel memcpy, ma l'indirizzo quando metto il jmp è completmanete diverso
	// a sto punto verificare l'address prima del gateaddr e vedere se muta o cosa
	 
	// Jump back to the next instruction after the trampoline has executed
	DWORD relJmpBack = (beginHook + minBytes) - ((uintptr_t)gateAddr + pushaLen) - 5;
	*(BYTE *)(gateAddr) = 0xE9;
	*(DWORD *)((uintptr_t)gateAddr + 1) = relJmpBack;

	std::cout << "bH: " << std::hex << beginHook << std::endl;
	std::cout << "dst: " << std::hex << beginHook+minBytes << std::endl;
	std::cout << "gateAddr: " << std::hex << gateAddr << std::endl;
	std::cout << "from: " << std::hex << (uintptr_t)gateAddr + pushaLen << std::endl;
	std::cout << "offset: " << std::hex << relJmpBack << std::endl;

	return (uintptr_t)gateAddr;
}

// Returns a pointer to the newly allocated gateway, near to the startHkFunc supplied.
/*
	Offset might be 0
*/
LPVOID ConstructGateway(
	_In_ uintptr_t startHkFunc,
	_In_ void (*__register_saver)(),
	_In_opt_ DWORD offset,
	_In_ bool jmpBack
)
{
	/*
		sub/add rsp, 108h = 7 bytes
		push rax...registers = 1 byte per instruction -> 7 bytes
		push r8...r15 = 2 bytes per instruction -> 8*2 bytes
	*/
	constexpr int pushaSize = 7 + 8 * 2 + 7;

	// Try to allocate the gateway near enough
	// that's because we want to avoid having 64bit address displacement
	// (cause x64 jmps suck, lol)
	LPVOID gatewayAddress = AllocatePageNearAddress((void *)startHkFunc);
	if (gatewayAddress == NULL)
		return nullptr;

	// Get __pusha_64 | __popa_64 start address
	uintptr_t pushaJumpAddress = ResolveJumpAddress((BYTE *)__register_saver);
	// Copy its instructions into newly allocated gateway
	memcpy(gatewayAddress, (void *)pushaJumpAddress, pushaSize);
	// Calculate jump back offset
	uintptr_t relativeJmpBackAddress = NULL;
	relativeJmpBackAddress = (BYTE *)(startHkFunc - pushaSize) - (BYTE *)gatewayAddress;
	if (offset != 0)
		relativeJmpBackAddress += offset;

	if (jmpBack)
	{
		// Add the jump back; at the end of the gate
		*(BYTE*)((uintptr_t)gatewayAddress + pushaSize) = 0xE9;
		*(uintptr_t*)((uintptr_t)gatewayAddress + pushaSize + 1) = relativeJmpBackAddress;
	}

	// std::cout << "Gate addr: " << gatewayAddress << std::endl;

	return gatewayAddress;
}

void *AllocatePageNearAddress(void *targetAddr)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	const uint64_t PAGE_SIZE = sysInfo.dwPageSize;

	uint64_t startAddr = (uint64_t(targetAddr) & ~(PAGE_SIZE - 1)); // round down to nearest page boundary
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
			void *outAddr = VirtualAlloc((void *)highAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (outAddr)
				return outAddr;
		}

		if (lowAddr > minAddr)
		{
			void *outAddr = VirtualAlloc((void *)lowAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
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

uintptr_t ResolveJumpAddress(BYTE *funcPtr)
{
	BYTE *instruction = (BYTE *)funcPtr;

	if (*instruction != 0xE9)
	{
		std::cout << "[Warning]: No jump detected. This is probably an issue." << std::endl;
		return (uintptr_t) funcPtr;
	}

	DWORD offset = *(DWORD *)(instruction + 1);
	uintptr_t jumpAddress = (uintptr_t)funcPtr + offset + 5;

	return jumpAddress;
}

// unique_ptr<VTEntrySwapper> Trampoline64(
// 	BYTE *hkFunc,
// 	BYTE **origFunc,
// 	DWORD offset)
// {
// 	uintptr_t beginHook = reinterpret_cast<uintptr_t>(hkFunc) + offset;

// }

// unique_ptr<VTEntrySwapper> Trampoline64(BYTE* hkFunc, BYTE** origFunc)
// {
// 	// Calculate relative address to jump back
// 	uintptr_t hookJumpAddress = ResolveJumpAddress((BYTE*)hkFunc);

// 	DWORD offsetToEndHook = GetEndOfHook((BYTE*)hookJumpAddress, -1, origFunc);
// 	uintptr_t hookEndJumpAddress = hookJumpAddress + offsetToEndHook;

// 	// Build __pusha_64 and __popa_64 gateways
// 	LPVOID pushaGateway = ConstructGateway(
// 		hookJumpAddress,
// 		__pusha_64,
// 		NULL
// 	);
// 	LPVOID popaGateway = ConstructGateway(
// 		hookEndJumpAddress,
// 		__popa_64,
// 		*origFunc
// 	);

// 	// Modify hkFunc to jump to the first gateway and adjust the prologue
// 	DetourHookedFunction(hookJumpAddress, pushaGateway);

// 	PatchEpilogue(hookEndJumpAddress, popaGateway, (LPVOID)origFunc);

// 	return std::make_unique<VTEntrySwapper>(VTEntrySwapper());
// }

// void PatchEpilogue(uintptr_t startHkFunc, LPVOID gatewayAddress, LPVOID origFunc)
// {
// 	DWORD offsetToEndHook = GetEndOfHook(
// 		(BYTE*)startHkFunc,
// 		1000,
// 		origFunc
// 	);

// 	constexpr int sizeof86relJmp = 5;
// 	DWORD prot;
// 	VirtualProtect(
// 		reinterpret_cast<LPVOID>(startHkFunc + offsetToEndHook),
// 		sizeof86relJmp,
// 		PAGE_EXECUTE_READWRITE,
// 		&prot
// 	);
// 	DWORD relPopaAddress = (uintptr_t)gatewayAddress - startHkFunc - 5;
// 	*(BYTE*)(startHkFunc + offsetToEndHook) = 0xE9;
// 	*(DWORD*)((BYTE*)startHkFunc + offsetToEndHook + 1) = relPopaAddress;
// 	VirtualProtect(
// 		reinterpret_cast<LPVOID>(startHkFunc + offsetToEndHook),
// 		sizeof86relJmp,
// 		prot,
// 		&prot
// 	);
// }

// void DetourHookedFunction(uintptr_t startHkFunc, LPVOID gatewayAddress)
// {
// 	// Find the end of the prologue, delimited by the first lea instruction.
// 	// This could be inaccurate, but for our needs it's good enough.
// 	DWORD offsetToEndPrologue = FindPrologue((BYTE*)startHkFunc, 200);
// 	// NOP the prologue at the start of the hookFunction and change its memory protection
// 	DWORD prot;
// 	VirtualProtect((LPVOID)startHkFunc, offsetToEndPrologue, PAGE_EXECUTE_READWRITE, &prot);
// 	memset((void*)startHkFunc, 0x90, offsetToEndPrologue);
// 	// Write the jump to the gate at the start of the hooked function
// 	uintptr_t offsetToGate = (uintptr_t)gatewayAddress - startHkFunc - 5;
// 	*(BYTE*)startHkFunc = 0xE9;
// 	*(DWORD*)((BYTE*)startHkFunc + 1) = offsetToGate;
// 	VirtualProtect((LPVOID)startHkFunc, offsetToEndPrologue, prot, &prot);

// 	//std::cout << "Offset to gate " << std::hex << offsetToGate << std::endl;
// 	//std::cout << "Addy: " << std::hex << (uintptr_t)gatewayAddress << std::endl;
// 	//std::cout << "pusha64: " << std::hex << __pusha_64 << std::endl;
// 	std::cout << "hkFunc: " << std::hex << (uintptr_t)startHkFunc << std::endl;
// 	//std::cout << "offset to end prologue: " << std::hex << offsetToEndPrologue << std::endl;
// }