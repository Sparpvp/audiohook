#pragma once
#include <Windows.h>

// ---- Type Definitions ---
typedef HRESULT(__stdcall* tRenderGetBuffer)(
    _In_ UINT32 NumFramesRequested,
    _Out_ BYTE** ppData
);

typedef HRESULT(__stdcall* tRenderReleaseBuffer)(
    _In_ UINT32 NumFramesWritten,
    _In_ DWORD dwFlags
);

typedef HRESULT(__stdcall* tCaptureGetBuffer)(
    _Out_ BYTE** ppData,
    _Out_ UINT32* pNumFramesToRead,
    _Out_ DWORD* pdwFlags,
    _Out_ UINT64* pu64DevicePosition,
    _Out_ UINT64* pu64QPCPosition
);

typedef HRESULT(__stdcall* tCaptureReleaseBuffer)(
    _In_ UINT32 NumFramesRead
);