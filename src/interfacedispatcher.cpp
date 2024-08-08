#include <iostream>
#include <Windows.h>

#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>

#include "interfacedispatcher.h"

IAudioRenderClient* InterfaceDispatcher::GetRenderCOMInterface()
{
    HRESULT beef = CoInitialize(NULL);
    if (FAILED(beef))
        return nullptr;

    IMMDeviceEnumerator* pEnumerator = NULL;
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    beef = CoCreateInstance(
        CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        (void**)&pEnumerator
    );
    if (FAILED(beef))
        return nullptr;

    IMMDevice* device = NULL;
    pEnumerator->GetDefaultAudioEndpoint(
        EDataFlow::eRender,
        ERole::eConsole,
        &device
    );

    const IID IID_IAudioClient = __uuidof(IAudioClient);
    IAudioClient* IAudioClientPtr = NULL;
    device->Activate(
        IID_IAudioClient,
        CLSCTX_ALL,
        NULL,
        (void**)&IAudioClientPtr
    );

    WAVEFORMATEX* wfxPtr;
    IAudioClientPtr->GetMixFormat(&wfxPtr);
    beef = IAudioClientPtr->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, wfxPtr, 0);
    if (FAILED(beef)) {
        std::cout << "Failed to initialize AudioClientPtr\n";
    }

    std::cout << "IAudioClientPtr: " << std::hex << IAudioClientPtr << std::endl;

    IAudioRenderClient* renderInterfacePtr; // HABEMUS POINTER
    beef = IAudioClientPtr->GetService(
        __uuidof(IAudioRenderClient),
        (void**)&renderInterfacePtr
    );
    if (FAILED(beef))
        std::cout << "Failed to get AudioRenderClient service: " << std::hex << beef << std::endl;

    return renderInterfacePtr;
}

IAudioCaptureClient* InterfaceDispatcher::GetCaptureCOMInterface()
{
    HRESULT beef = CoInitialize(NULL);
    if (FAILED(beef))
        return nullptr;

    IMMDeviceEnumerator* pEnumerator = NULL;
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    beef = CoCreateInstance(
        CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        (void**)&pEnumerator
    );
    if (FAILED(beef))
        return nullptr;

    IMMDevice* device = NULL;
    pEnumerator->GetDefaultAudioEndpoint(
        EDataFlow::eCapture,
        ERole::eConsole,
        &device
    );

    const IID IID_IAudioClient = __uuidof(IAudioClient);
    IAudioClient* IAudioClientPtr = NULL;
    device->Activate(
        IID_IAudioClient,
        CLSCTX_ALL,
        NULL,
        (void**)&IAudioClientPtr
    );

    WAVEFORMATEX* wfxPtr;
    IAudioClientPtr->GetMixFormat(&wfxPtr);
    beef = IAudioClientPtr->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, wfxPtr, 0);
    if (FAILED(beef)) {
        std::cout << "Failed to initialize AudioClientPtr\n";
    }

    std::cout << "IAudioClientPtr: " << std::hex << IAudioClientPtr << std::endl;

    IAudioCaptureClient* captureInterfacePtr; // HABEMUS POINTER
    beef = IAudioClientPtr->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&captureInterfacePtr
    );
    if (FAILED(beef))
        std::cout << "Failed to get AudioCaptureClient service: " << std::hex << beef << std::endl;

    return captureInterfacePtr;
}