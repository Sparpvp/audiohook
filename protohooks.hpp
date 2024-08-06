#pragma once
#include <iostream>

#include "templatefuncs.h"
#include "ptrglobal.hpp"
#include "registersaver.h"

HRESULT __stdcall g_fnHkRenderGetBuffer(
    _In_ UINT32 NumFramesRequested,
    _Out_ BYTE** ppData
)
{
    // Our code
    HRESULT status = g_oRenderGetBuffer(NumFramesRequested, ppData);

    //__pusha_64();
    OutputDebugStringA("[RENDER] GetBuffer");
    std::cout << "[RENDER] GetBuffer Hooked from vtable!" << std::endl;
    //__popa_64();

    // Trampoline
    return status;
}

HRESULT __stdcall g_fnHkRenderReleaseBuffer(
    _In_ UINT32 NumFramesWritten,
    _In_ DWORD dwFlags
)
{
    std::cout << "[RENDER] ReleaseBuffer Hooked from vtable!" << std::endl;
    OutputDebugStringA("[RENDER] ReleaseBuffer");

    return g_oRenderReleaseBuffer(NumFramesWritten, dwFlags);
}

HRESULT __stdcall g_fnHkCaptureGetBuffer(
    _Out_ BYTE** ppData,
    _Out_ UINT32* pNumFramesToRead,
    _Out_ DWORD* pdwFlags,
    _Out_ UINT64* pu64DevicePosition,
    _Out_ UINT64* pu64QPCPosition
)
{
    std::cout << "[CAPTURE] GetBuffer Hooked from vtable!" << std::endl;
    OutputDebugStringA("[CAPTURE] GetBuffer");

    return g_oCaptureGetBuffer(ppData, pNumFramesToRead, pdwFlags, pu64DevicePosition, pu64QPCPosition);
}

HRESULT __stdcall g_fnHkCaptureReleaseBuffer(_In_ UINT32 NumFramesRead)
{
    std::cout << "[CAPTURE] ReleaseBuffer Hooked from vtable!" << std::endl;
    OutputDebugStringA("[CAPTURE] ReleaseBuffer");

    return g_oCaptureReleaseBuffer(NumFramesRead);
}