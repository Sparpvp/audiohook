#pragma once

#include <audioclient.h>

class VTables
{
public:
	void** GetRenderClientVTable(IAudioRenderClient* audioRenderClient);
	void** GetCaptureClientVTable(IAudioCaptureClient* audioCaptureClient);
};