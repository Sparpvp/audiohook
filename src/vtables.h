#pragma once

#include <audioclient.h>

namespace VTables
{
	void** GetRenderClientVTable(IAudioRenderClient* audioRenderClient);
	void** GetCaptureClientVTable(IAudioCaptureClient* audioCaptureClient);
}