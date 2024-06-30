#include <iostream>
#include <Windows.h>

#include "hooker.h"
#include "templatefuncs.h"
#include "trampoline.h"
#include "ptrglobal.hpp"

/*
    At vTable[0], vTable[1] we have the same functions in both interfaces.
    Hence, we can safely abstract and not care about a type-specific implementation here.
*/

template <typename T>
void VTEntrySwapper::HelperGetBuffer(void** vTablePtr, const T ourHookFunc)
{
    constexpr int lpvoidSize = sizeof(LPVOID);
    DWORD prot;
    VirtualProtect(vTablePtr, lpvoidSize, PAGE_EXECUTE_READWRITE, &prot);
    vTablePtr[0] = reinterpret_cast<LPVOID>(ourHookFunc);
    VirtualProtect(vTablePtr, lpvoidSize, prot, &prot);
}

template <typename T>
void VTEntrySwapper::HelperReleaseBuffer(void** vTablePtr, const T ourHookFunc)
{
    constexpr int lpvoidSize = sizeof(LPVOID);
    DWORD prot;
    VirtualProtect(vTablePtr + lpvoidSize, lpvoidSize, PAGE_EXECUTE_READWRITE, &prot);
    vTablePtr[1] = reinterpret_cast<LPVOID>(ourHookFunc);
    VirtualProtect(vTablePtr + lpvoidSize, lpvoidSize, prot, &prot);
}

// Initialize release global origin pointers to construct the class.
RenderAudioClientHooker::RenderAudioClientHooker(void** vTablePtr) 
    : m_vTablePtr(vTablePtr)
{
    g_oRenderGetBuffer = (tRenderGetBuffer)m_vTablePtr[0];
    g_oRenderReleaseBuffer = (tRenderReleaseBuffer)m_vTablePtr[1];
}

// Initialize capture global origin pointers to construct the class.
CaptureAudioClientHooker::CaptureAudioClientHooker(void** vTablePtr)
    : m_vTablePtr(vTablePtr)
{
    g_oCaptureGetBuffer = (tCaptureGetBuffer)m_vTablePtr[0];
    g_oCaptureReleaseBuffer = (tCaptureReleaseBuffer)m_vTablePtr[1];
}

// Swap Render GetBuffer Function VTable entry with our hooked one
void RenderAudioClientHooker::InstallHook(const tRenderGetBuffer ourHookFunc, tRenderGetBuffer* origFunc)
{
    unique_ptr<VTEntrySwapper> vts = Trampoline64(
        reinterpret_cast<BYTE*>(ourHookFunc),
        reinterpret_cast<BYTE**>(origFunc)
    );
    vts->HelperGetBuffer(m_vTablePtr, ourHookFunc);
}

// Swap Render ReleaseBuffer Function VTable entry with our hooked one
void RenderAudioClientHooker::InstallHook(const tRenderReleaseBuffer ourHookFunc, tRenderReleaseBuffer* origFunc)
{
    unique_ptr<VTEntrySwapper> vts = Trampoline64(
        reinterpret_cast<BYTE*>(ourHookFunc),
        reinterpret_cast<BYTE**>(origFunc)
    );
    vts->HelperReleaseBuffer(m_vTablePtr, ourHookFunc);
}

// Swap Capture GetBuffer VTable entry with our hooked one
void CaptureAudioClientHooker::InstallHook(const tCaptureGetBuffer ourHookFunc, tCaptureGetBuffer* origFunc)
{
    unique_ptr<VTEntrySwapper> vts = Trampoline64(
        reinterpret_cast<BYTE*>(ourHookFunc),
        reinterpret_cast<BYTE**>(origFunc)
    );
    vts->HelperGetBuffer(m_vTablePtr, ourHookFunc);
}

// Swap Capture ReleaseBuffer VTable entry with our hooked one
void CaptureAudioClientHooker::InstallHook(const tCaptureReleaseBuffer ourHookFunc, tCaptureReleaseBuffer* origFunc)
{
    unique_ptr<VTEntrySwapper> vts = Trampoline64(
        reinterpret_cast<BYTE*>(ourHookFunc),
        reinterpret_cast<BYTE**>(origFunc)
    );
    vts->HelperReleaseBuffer(m_vTablePtr, ourHookFunc);
}