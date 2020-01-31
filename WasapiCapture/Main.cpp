//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

#define AUDIO_CAPTURE_FILE		L"Capture.wav"
#define REFTIMES_PER_SEC		10000000
#define REFTIMES_PER_MILLISEC	10000
#define MAX_LOOP_BEFORE_STOP	20

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

HRESULT RecordAudioStream();

void main()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if(SUCCEEDED(hr))
	{
		LOG_HRESULT(RecordAudioStream());

		CoUninitialize();
	}
}

HRESULT RecordAudioStream()
{
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	CMFWaveWriter cMFWaveWriter;
	UINT32 uiFileLength = 0;
	BOOL bExtensibleFormat = FALSE;

	try
	{
		IF_FAILED_THROW(CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator));
		IF_FAILED_THROW(pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice));
		IF_FAILED_THROW(pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient));

		IF_FAILED_THROW(pAudioClient->GetMixFormat(&pwfx));

		switch(pwfx->wFormatTag)
		{
			case WAVE_FORMAT_PCM:
				TRACE((L"WAVE_FORMAT_PCM"));
				break;

			case WAVE_FORMAT_IEEE_FLOAT:
				TRACE((L"WAVE_FORMAT_IEEE_FLOAT"));
				break;

			case WAVE_FORMAT_EXTENSIBLE:
				TRACE((L"WAVE_FORMAT_EXTENSIBLE"));
				bExtensibleFormat = TRUE;

				WAVEFORMATEXTENSIBLE *pWaveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(pwfx);

				if(pWaveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
				{
					TRACE((L"KSDATAFORMAT_SUBTYPE_PCM"));
				}
				else if(pWaveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
				{
					TRACE((L"KSDATAFORMAT_SUBTYPE_IEEE_FLOAT"));
				}
				break;
		}

		IF_FAILED_THROW(pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL));
		IF_FAILED_THROW(pAudioClient->GetBufferSize(&bufferFrameCount));
		IF_FAILED_THROW(pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient));

		IF_FAILED_THROW(cMFWaveWriter.Initialize(AUDIO_CAPTURE_FILE, bExtensibleFormat) ? S_OK : E_FAIL);

		hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

		IF_FAILED_THROW(pAudioClient->Start());

		BOOL bDone = FALSE;
		UINT32 packetLength = 0;
		UINT32 numFramesAvailable;
		BYTE* pData;
		DWORD flags;
		int iLoop = 0;

		while(bDone == FALSE)
		{
			Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);

			IF_FAILED_THROW(pCaptureClient->GetNextPacketSize(&packetLength));

			while(packetLength != 0)
			{
				IF_FAILED_THROW(pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL));

				if(flags & AUDCLNT_BUFFERFLAGS_SILENT)
				{
					TRACE((L"AUDCLNT_BUFFERFLAGS_SILENT"));
					break;
				}

				if(flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
				{
					TRACE((L"AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY"));
				}

				if(flags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR)
				{
					TRACE((L"AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR"));
				}

				//TRACE((L"numFramesAvailable : %u", numFramesAvailable));

				assert(packetLength == numFramesAvailable);

				IF_FAILED_THROW(cMFWaveWriter.WriteWaveData(pData, numFramesAvailable * pwfx->nBlockAlign) ? S_OK : E_FAIL);

				uiFileLength += numFramesAvailable;

				IF_FAILED_THROW(pCaptureClient->ReleaseBuffer(numFramesAvailable));

				IF_FAILED_THROW(pCaptureClient->GetNextPacketSize(&packetLength));
			}

			if(iLoop++ == MAX_LOOP_BEFORE_STOP)
				bDone = TRUE;
		}
	}
	catch(HRESULT){}

	TRACE((L"uiFileLength : %u", uiFileLength));

	if(hr == S_OK && pwfx != NULL)
		cMFWaveWriter.FinalizeHeader(pwfx, uiFileLength, bExtensibleFormat);

	if(pAudioClient)
	{
		LOG_HRESULT(pAudioClient->Stop());
		SAFE_RELEASE(pAudioClient);
	}

	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pCaptureClient);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);

	return hr;
}