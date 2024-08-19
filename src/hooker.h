#pragma once

#include "templatefuncs.h"

class VTEntrySwapper
{
private:
    VTEntrySwapper() {}
    friend std::unique_ptr<VTEntrySwapper> Trampoline64(
        BYTE* hkFunc,
        BYTE** origFunc,
        void (*__register_saver)(),
        DWORD offset
    );

public:
    template <typename T>
    void HelperGetBuffer(void **vTablePtr, const T ourHookFunc);
    template <typename T>
    void HelperReleaseBuffer(void **vTablePtr, const T ourHookFunc);
};

// Enforce original (Render)Get/ReleaseBuffer method to be initialized
// to orig pointer before applying the hook
class RenderAudioClientHooker
{
public:
    RenderAudioClientHooker(void **vTablePtr);

    void InstallHook(const tRenderGetBuffer ourHookFunc, tRenderGetBuffer *origFunc);
    void InstallHook(const tRenderReleaseBuffer ourHookFunc, tRenderReleaseBuffer *origFunc);

private:
    void **m_vTablePtr = nullptr;
};

// Enforce original (Capture)Get/ReleaseBuffer method to be initialized
// to orig pointer before applying the hook
class CaptureAudioClientHooker
{
public:
    CaptureAudioClientHooker(void **vTablePtr);

    void InstallHook(const tCaptureGetBuffer ourHookFunc, tCaptureGetBuffer *origFunc);
    void InstallHook(const tCaptureReleaseBuffer ourHookFunc, tCaptureReleaseBuffer *origFunc);

private:
    void **m_vTablePtr = nullptr;
};

// template <class T>
// template <typename... Ts>
// class HookSet
// {
// public:
//     HookSet(T protoHook, void (*hookCode)(), Ts... moreHookCalls)
//         : m_protoHook(protoHook),
//           m_hookCode(hookCode) {}

// private:
//     T m_protoHook;
//     void (*m_hookCode)();
// };