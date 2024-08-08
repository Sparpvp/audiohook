#pragma once

#include <audioclient.h>

namespace InterfaceDispatcher
{
	IAudioRenderClient* GetRenderCOMInterface();
	IAudioCaptureClient* GetCaptureCOMInterface();
}