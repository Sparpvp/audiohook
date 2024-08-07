#include <iostream>
#include <Windows.h>

#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>

#include "templatefuncs.h"
#include "vtables.h"
#include "hooker.h"
#include "protohooks.hpp"
#include "interfacedispatcher.h"
#include "trampoline.h"

void RunHooking()
{
    IAudioRenderClient* mockAudioRenderClient = InterfaceDispatcher::GetRenderCOMInterface();
    IAudioCaptureClient* mockAudioCaptureClient = InterfaceDispatcher::GetCaptureCOMInterface();

    VTables vtb;
    void** renderVTablePtr = vtb.GetRenderClientVTable(mockAudioRenderClient);
    void** captureVTablePtr = vtb.GetCaptureClientVTable(mockAudioCaptureClient);

    RenderAudioClientHooker renderHooker = RenderAudioClientHooker(renderVTablePtr);
    renderHooker.InstallHook(
        g_fnHkRenderGetBuffer, 
        &g_oRenderGetBuffer
    );
    /*renderHooker.InstallHook(
        g_fnHkRenderReleaseBuffer,
        &g_oRenderReleaseBuffer
    );*/

    /*CaptureAudioClientHooker captureHooker = CaptureAudioClientHooker(captureVTablePtr);
    captureHooker.InstallHook(
        g_fnHkCaptureGetBuffer,
        &g_oCaptureGetBuffer
    );
    captureHooker.InstallHook(
        g_fnHkCaptureReleaseBuffer,
        &g_oCaptureReleaseBuffer
    );*/
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        OutputDebugStringA("Begin Hooking");
        RunHooking();
        OutputDebugStringA("Everything's hooked.");
    // Care about process detach -> return or idk, cleanup maybe
    }
    return TRUE;
}

