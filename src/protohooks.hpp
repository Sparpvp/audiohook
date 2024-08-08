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

    OutputDebugStringA("[RENDER] GetBuffer");
    std::cout << "[RENDER] GetBuffer Hooked from vtable!" << std::endl;

    // Trampoline
    return status;
}

HRESULT __stdcall g_fnHkRenderReleaseBuffer(
    _In_ UINT32 NumFramesWritten,
    _In_ DWORD dwFlags
)
{
    OutputDebugStringA("[RENDER] ReleaseBuffer");
    std::cout << "[RENDER] ReleaseBuffer Hooked from vtable!" << std::endl;

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
    OutputDebugStringA("[CAPTURE] GetBuffer");
    std::cout << "[CAPTURE] GetBuffer Hooked from vtable!" << std::endl;

    return g_oCaptureGetBuffer(ppData, pNumFramesToRead, pdwFlags, pu64DevicePosition, pu64QPCPosition);
}

HRESULT __stdcall g_fnHkCaptureReleaseBuffer(_In_ UINT32 NumFramesRead)
{
    OutputDebugStringA("[CAPTURE] ReleaseBuffer");
    std::cout << "[CAPTURE] ReleaseBuffer Hooked from vtable!" << std::endl;

    return g_oCaptureReleaseBuffer(NumFramesRead);
}