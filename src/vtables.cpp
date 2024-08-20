#include <iostream>
#include <audioclient.h>

#include "vtables.h"

/*
    1) audioRenderClient = Pointer alla interfaccia | uint64_t[]**
    2) *audioRenderClient = Pointer alla vtable | uint64_t[]*
    3) **audioRenderClient = vtable | uint64_t[]
    4) **audioRenderClient[x] = method pointer | uint64_t aka, ***audioRenderClient.
*/
void** VTables::GetRenderClientVTable(IAudioRenderClient* audioRenderClient)
{
    //std::cout << "ptr to Render Interface: " << std::hex << audioRenderClient << std::endl;
    //std::cout << "ptr to Render vTable: " << std::hex << captureVTablePtr << std::endl;
    //std::cout << "ptr to RenderGetBuffer: " << std::hex << captureVTablePtr[0] << std::endl;

    void** captureVTablePtr = *reinterpret_cast<void***>(audioRenderClient);

    return captureVTablePtr;
}

/*
    1) audioCaptureClient = Pointer alla interfaccia | uint64_t[]**
    2) *audioCaptureClient = Pointer alla vtable | uint64_t[]*
    3) **audioCaptureClient = vtable | uint64_t[]
    4) **audioCaptureClient[x] = method pointer | uint64_t aka, ***audioCaptureClient.
*/
void** VTables::GetCaptureClientVTable(IAudioCaptureClient* audioCaptureClient)
{
    void** captureVTablePtr = *reinterpret_cast<void***>(audioCaptureClient);

    return captureVTablePtr;
}
