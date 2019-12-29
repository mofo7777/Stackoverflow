//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

// Replace Smartphone.mp4 with your video
#define MP4_SOURCE_VIDEO_MEDIA_FILE L"Smartphone.mp4"
#define MP4_FINAL_VIDEO_MEDIA_FILE L"final.mp4"

#define OUTPUT_READER_VIDEO_TYPE	MFVideoFormat_NV12
#define OUTPUT_READER_AUDIO_TYPE	MFAudioFormat_PCM
#define AVG_BITRATE_VALUE			20000000

struct MEDIA_INDEX
{
	DWORD dwReaderVideoStreamIndex;
	DWORD dwReaderAudioStreamIndex;
	DWORD dwWriterVideoStreamIndex;
	DWORD dwWriterAudioStreamIndex;
};

HRESULT ProcessEncoding(LPCWSTR, LPCWSTR);
HRESULT CreateSourceReader(IMFSourceReader**, LPCWSTR);
HRESULT GetSourceReaderIndexes(IMFSourceReader*, MEDIA_INDEX&);
HRESULT SetupSourceReaderMediaType(IMFSourceReader*, IMFMediaType**, IMFMediaType**, const MEDIA_INDEX);
HRESULT SetupTransform(IMFMediaType*, IMFTransform**);
HRESULT CreateSinkWriter(IMFSinkWriter**, LPCWSTR);
HRESULT SetupSinkWriterMediaType(IMFSinkWriter*, IMFMediaType*, IMFMediaType*, MEDIA_INDEX&);
HRESULT CreateOutputVideoType(IMFMediaType**, IMFMediaType*);
HRESULT CreateOutputAudioType(IMFMediaType**, IMFMediaType*);
HRESULT EncodeFile(IMFSourceReader*, IMFTransform*, IMFSinkWriter*, const MEDIA_INDEX&);
HRESULT ProcessStabilization(IMFTransform*, IMFSample*, IMFSample**);
HRESULT CheckNewMediaType(IMFSourceReader*, IMFTransform*, IMFSinkWriter*, const DWORD, const DWORD);
void TraceWriteSample(IMFSample*, const BOOL);

void main()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if(SUCCEEDED(hr))
	{
		hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);

		if(SUCCEEDED(hr))
		{
			ProcessEncoding(MP4_SOURCE_VIDEO_MEDIA_FILE, MP4_FINAL_VIDEO_MEDIA_FILE);

			LOG_HRESULT(MFShutdown());
		}

		CoUninitialize();
	}
}

HRESULT ProcessEncoding(LPCWSTR wszVideoFile, LPCWSTR wszFinalVideoFile)
{
	HRESULT hr = S_OK;
	IMFSourceReader* pSourceReader = NULL;
	IMFMediaType* pInputVideoType = NULL;
	IMFMediaType* pInputAudioType = NULL;
	IMFSinkWriter* pSinkWriter = NULL;
	IMFTransform* pTransform = NULL;
	MEDIA_INDEX MediaIndex;

	try
	{
		// SourceReader
		IF_FAILED_THROW(CreateSourceReader(&pSourceReader, wszVideoFile));
		IF_FAILED_THROW(GetSourceReaderIndexes(pSourceReader, MediaIndex));
		IF_FAILED_THROW(SetupSourceReaderMediaType(pSourceReader, &pInputVideoType, &pInputAudioType, MediaIndex));
		// Transform
		IF_FAILED_THROW(SetupTransform(pInputVideoType, &pTransform));
		// SinkWriter
		IF_FAILED_THROW(CreateSinkWriter(&pSinkWriter, wszFinalVideoFile));
		IF_FAILED_THROW(SetupSinkWriterMediaType(pSinkWriter, pInputVideoType, pInputAudioType, MediaIndex));
		// Encoding
		IF_FAILED_THROW(EncodeFile(pSourceReader, pTransform, pSinkWriter, MediaIndex));
	}
	catch(HRESULT){}

	if(pTransform)
	{
		LOG_HRESULT(pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL));
		LOG_HRESULT(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, NULL));
		LOG_HRESULT(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, NULL));
		SAFE_RELEASE(pTransform);
	}

	SAFE_RELEASE(pInputAudioType);
	SAFE_RELEASE(pInputVideoType);
	SAFE_RELEASE(pSourceReader);

	return hr;
}

HRESULT CreateSourceReader(IMFSourceReader** ppSourceReader, LPCWSTR wszVideoFile)
{
	HRESULT hr = S_OK;
	IMFSourceReader* pSourceReader = NULL;
	IMFAttributes* pAttributes = NULL;

	try
	{
		IF_FAILED_THROW(MFCreateAttributes(&pAttributes, 3));
		IF_FAILED_THROW(pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE));
		IF_FAILED_THROW(pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
		IF_FAILED_THROW(pAttributes->SetUINT32(MF_LOW_LATENCY, FALSE));

		IF_FAILED_THROW(MFCreateSourceReaderFromURL(wszVideoFile, pAttributes, &pSourceReader));

		*ppSourceReader = pSourceReader;
		pSourceReader->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pAttributes);
	SAFE_RELEASE(pSourceReader);

	return hr;
}

HRESULT GetSourceReaderIndexes(IMFSourceReader* pSourceReader, MEDIA_INDEX& MediaIndex)
{
	HRESULT hr = S_OK;
	IMFMediaType *pNativeType = NULL;
	DWORD dwStreamIndex = 0;
	GUID majorType;

	MediaIndex.dwReaderVideoStreamIndex = (DWORD)MF_SOURCE_READER_INVALID_STREAM_INDEX;
	MediaIndex.dwReaderAudioStreamIndex = (DWORD)MF_SOURCE_READER_INVALID_STREAM_INDEX;

	do
	{
		try
		{
			IF_FAILED_THROW(pSourceReader->GetNativeMediaType(dwStreamIndex, 0, &pNativeType));
			IF_FAILED_THROW(pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType));

			if(MediaIndex.dwReaderVideoStreamIndex == MF_SOURCE_READER_INVALID_STREAM_INDEX && majorType == MFMediaType_Video)
			{
				MediaIndex.dwReaderVideoStreamIndex = dwStreamIndex;
			}
			else if(MediaIndex.dwReaderAudioStreamIndex == MF_SOURCE_READER_INVALID_STREAM_INDEX && majorType == MFMediaType_Audio)
			{
				MediaIndex.dwReaderAudioStreamIndex = dwStreamIndex;
			}

			++dwStreamIndex;
		}
		catch(HRESULT){}

		SAFE_RELEASE(pNativeType);

		if(MediaIndex.dwReaderVideoStreamIndex != MF_SOURCE_READER_INVALID_STREAM_INDEX && MediaIndex.dwReaderAudioStreamIndex != MF_SOURCE_READER_INVALID_STREAM_INDEX)
			break;
	}
	while(hr == S_OK);

	return hr;
}

HRESULT SetupSourceReaderMediaType(IMFSourceReader* pSourceReader, IMFMediaType** ppInputVideoType, IMFMediaType** ppInputAudioType, const MEDIA_INDEX MediaIndex)
{
	HRESULT hr = S_OK;
	IMFMediaType* pInputType = NULL;

	try
	{
		// Video
		IF_FAILED_THROW(MFCreateMediaType(&pInputType));
		IF_FAILED_THROW(pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(pInputType->SetGUID(MF_MT_SUBTYPE, OUTPUT_READER_VIDEO_TYPE));

		IF_FAILED_THROW(pSourceReader->SetCurrentMediaType(MediaIndex.dwReaderVideoStreamIndex, NULL, pInputType));
		IF_FAILED_THROW(pSourceReader->GetCurrentMediaType(MediaIndex.dwReaderVideoStreamIndex, ppInputVideoType));

		TRACE_NO_END_LINE((L"Input Video Media Type :"));
		LogMediaType(*ppInputVideoType);

		// Audio
		IF_FAILED_THROW(pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		IF_FAILED_THROW(pInputType->SetGUID(MF_MT_SUBTYPE, OUTPUT_READER_AUDIO_TYPE));

		IF_FAILED_THROW(pSourceReader->SetCurrentMediaType(MediaIndex.dwReaderAudioStreamIndex, NULL, pInputType));
		IF_FAILED_THROW(pSourceReader->GetCurrentMediaType(MediaIndex.dwReaderAudioStreamIndex, ppInputAudioType));

		TRACE_NO_END_LINE((L"Input Audio Media Type :"));
		LogMediaType(*ppInputAudioType);
	}
	catch(HRESULT){}

	SAFE_RELEASE(pInputType);

	return hr;
}

HRESULT SetupTransform(IMFMediaType* pMediaType, IMFTransform** ppTransform)
{
	HRESULT hr = S_OK;
	IMFTransform* pTransform = NULL;
	IMFAttributes* pAttributes = NULL;
	DWORD dwInputStream;
	DWORD dwOutputStream;

	try
	{
		IF_FAILED_THROW(CoCreateInstance(CLSID_CMSVideoDSPMFT, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, reinterpret_cast<void**>(&pTransform)));
		IF_FAILED_THROW(pTransform->GetStreamCount(&dwInputStream, &dwOutputStream));
		assert(dwInputStream == 1 && dwOutputStream == 1);
		//IF_FAILED_THROW(pTransform->GetStreamIDs(1, &dwInputStream, 1, &dwOutputStream));
		//assert(dwInputStream == 0 && dwOutputStream == 0);

		IF_FAILED_THROW(pTransform->SetInputType(0, pMediaType, 0));
		IF_FAILED_THROW(pTransform->SetOutputType(0, pMediaType, 0));
		IF_FAILED_THROW(pTransform->GetAttributes(&pAttributes));
		// MFVideoDSPMode_Passthrough MFVideoDSPMode_Stabilization
		IF_FAILED_THROW(pAttributes->SetUINT32(MF_VIDEODSP_MODE, MFVideoDSPMode_Stabilization));
		IF_FAILED_THROW(pAttributes->SetUINT32(MF_LOW_LATENCY, FALSE));
		//IF_FAILED_THROW(pAttributes->SetUINT32(MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT, 1));

		*ppTransform = pTransform;
		pTransform->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pAttributes);
	SAFE_RELEASE(pTransform);

	return hr;
}

HRESULT CreateSinkWriter(IMFSinkWriter** ppSinkWriter, LPCWSTR wszFinalVideoFile)
{
	HRESULT hr = S_OK;
	IMFSinkWriter* pSinkWriter = NULL;
	IMFAttributes* pAttributes = NULL;

	try
	{
		IF_FAILED_THROW(MFCreateAttributes(&pAttributes, 2));
		IF_FAILED_THROW(pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
		IF_FAILED_THROW(pAttributes->SetUINT32(MF_LOW_LATENCY, FALSE));

		IF_FAILED_THROW(MFCreateSinkWriterFromURL(wszFinalVideoFile, NULL, pAttributes, &pSinkWriter));

		*ppSinkWriter = pSinkWriter;
		pSinkWriter->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pAttributes);
	SAFE_RELEASE(pSinkWriter);

	return hr;
}

HRESULT SetupSinkWriterMediaType(IMFSinkWriter* pSinkWriter, IMFMediaType* pInputVideoType, IMFMediaType* pInputAudioType, MEDIA_INDEX& MediaIndex)
{
	HRESULT hr = S_OK;
	IMFMediaType* pOutputVideoType = NULL;
	IMFMediaType* pOutputAudioType = NULL;

	MediaIndex.dwWriterVideoStreamIndex = (DWORD)MF_SOURCE_READER_INVALID_STREAM_INDEX;
	MediaIndex.dwWriterAudioStreamIndex = (DWORD)MF_SOURCE_READER_INVALID_STREAM_INDEX;

	try
	{
		// Video
		IF_FAILED_THROW(CreateOutputVideoType(&pOutputVideoType, pInputVideoType));

		TRACE_NO_END_LINE((L"Output Video Media Type :"));
		LogMediaType(pOutputVideoType);

		IF_FAILED_THROW(pSinkWriter->AddStream(pOutputVideoType, &MediaIndex.dwWriterVideoStreamIndex));
		IF_FAILED_THROW(pSinkWriter->SetInputMediaType(MediaIndex.dwWriterVideoStreamIndex, pInputVideoType, NULL));

		// Audio
		IF_FAILED_THROW(CreateOutputAudioType(&pOutputAudioType, pInputAudioType));

		TRACE_NO_END_LINE((L"Output Audio Media Type :"));
		LogMediaType(pOutputAudioType);

		IF_FAILED_THROW(pSinkWriter->AddStream(pOutputAudioType, &MediaIndex.dwWriterAudioStreamIndex));
		IF_FAILED_THROW(pSinkWriter->SetInputMediaType(MediaIndex.dwWriterAudioStreamIndex, pInputAudioType, NULL));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputAudioType);
	SAFE_RELEASE(pOutputVideoType);

	return hr;
}

HRESULT CreateOutputVideoType(IMFMediaType** ppMediatype, IMFMediaType* pInputMediatype)
{
	HRESULT hr = S_OK;
	IMFMediaType* pMediatype = NULL;
	UINT32 uiWidth;
	UINT32 uiHeight;
	UINT32 uiNumRate;
	UINT32 uiDenRate;
	UINT32 uiNumRatio;
	UINT32 uiDenRatio;
	//UINT32 uiBitrate;

	try
	{
		IF_FAILED_THROW(MFGetAttributeSize(pInputMediatype, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
		IF_FAILED_THROW(MFGetAttributeRatio(pInputMediatype, MF_MT_FRAME_RATE, &uiNumRate, &uiDenRate));
		IF_FAILED_THROW(MFGetAttributeRatio(pInputMediatype, MF_MT_PIXEL_ASPECT_RATIO, &uiNumRatio, &uiDenRatio));
		//IF_FAILED_THROW(pInputMediatype->GetUINT32(MF_MT_AVG_BITRATE, &uiBitrate));

		IF_FAILED_THROW(MFCreateMediaType(&pMediatype));
		IF_FAILED_THROW(pMediatype->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(pMediatype->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
		IF_FAILED_THROW(pMediatype->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
		IF_FAILED_THROW(MFSetAttributeSize(pMediatype, MF_MT_FRAME_SIZE, uiWidth, uiHeight));
		IF_FAILED_THROW(MFSetAttributeRatio(pMediatype, MF_MT_FRAME_RATE, uiNumRate, uiDenRate));
		IF_FAILED_THROW(MFSetAttributeRatio(pMediatype, MF_MT_PIXEL_ASPECT_RATIO, uiNumRatio, uiDenRatio));

		// Video quality is usually better with AVG_BITRATE_VALUE == 20000000
		IF_FAILED_THROW(pMediatype->SetUINT32(MF_MT_AVG_BITRATE, AVG_BITRATE_VALUE));
		//IF_FAILED_THROW(pMediatype->SetUINT32(MF_MT_AVG_BITRATE, uiBitrate));

		*ppMediatype = pMediatype;
		pMediatype->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediatype);

	return hr;
}

HRESULT CreateOutputAudioType(IMFMediaType** ppMediatype, IMFMediaType* pInputMediatype)
{
	HRESULT hr = S_OK;
	IMFMediaType* pMediatype = NULL;
	UINT32 uiChannels;
	UINT32 uiSamplePerSecond;
	UINT32 uiBitsPerSample;

	try
	{
		IF_FAILED_THROW(pInputMediatype->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uiChannels));
		IF_FAILED_THROW(pInputMediatype->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &uiSamplePerSecond));
		IF_FAILED_THROW(pInputMediatype->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &uiBitsPerSample));

		IF_FAILED_THROW(MFCreateMediaType(&pMediatype));
		IF_FAILED_THROW(pMediatype->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		IF_FAILED_THROW(pMediatype->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC));
		IF_FAILED_THROW(pMediatype->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, uiChannels));
		IF_FAILED_THROW(pMediatype->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, uiSamplePerSecond));
		IF_FAILED_THROW(pMediatype->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, uiBitsPerSample));

		*ppMediatype = pMediatype;
		pMediatype->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediatype);

	return hr;
}

HRESULT EncodeFile(IMFSourceReader* pSourceReader, IMFTransform* pTransform, IMFSinkWriter* pSinkWriter, const MEDIA_INDEX& MediaIndex)
{
	HRESULT hr;
	IMFSample* pInputSample = NULL;
	IMFSample* pOutputSample = NULL;
	DWORD dwReaderStreamIndex;
	DWORD dwWriterStreamIndex;
	DWORD dwFlags;
	LONGLONG llTimeStamp;
	BOOL bEndVideo = FALSE;
	BOOL bEndAudio = FALSE;

	IF_FAILED_RETURN(pTransform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, NULL));
	IF_FAILED_RETURN(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL));
	IF_FAILED_RETURN(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL));

	IF_FAILED_RETURN(pSinkWriter->BeginWriting());

	while(hr == S_OK)
	{
		LOG_HRESULT(hr = pSourceReader->ReadSample((DWORD)MF_SOURCE_READER_ANY_STREAM, 0, &dwReaderStreamIndex, &dwFlags, &llTimeStamp, &pInputSample));

		if(hr != S_OK)
		{
			break;
		}

		if(dwReaderStreamIndex != MediaIndex.dwReaderVideoStreamIndex && dwReaderStreamIndex != MediaIndex.dwReaderAudioStreamIndex)
		{
			SAFE_RELEASE(pInputSample);
			continue;
		}

		dwWriterStreamIndex = (dwReaderStreamIndex == MediaIndex.dwReaderVideoStreamIndex ? MediaIndex.dwWriterVideoStreamIndex : MediaIndex.dwWriterAudioStreamIndex);

		if(dwFlags & MF_SOURCE_READERF_ERROR)
		{
			break;
		}
		if(dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			if(bEndVideo && bEndAudio)
				break;

			if(dwReaderStreamIndex == MediaIndex.dwReaderVideoStreamIndex)
				bEndVideo = TRUE;
			else if(dwReaderStreamIndex == MediaIndex.dwReaderAudioStreamIndex)
				bEndAudio = TRUE;
		}
		if(dwFlags & MF_SOURCE_READERF_NEWSTREAM)
		{
			break;
		}
		if(dwFlags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
		{
			break;
		}
		if(dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
		{
			hr = CheckNewMediaType(pSourceReader, pTransform, pSinkWriter, dwReaderStreamIndex, dwWriterStreamIndex);
		}
		if(dwFlags & MF_SOURCE_READERF_STREAMTICK)
		{
			break;
		}
		if(dwFlags & MF_SOURCE_READERF_ALLEFFECTSREMOVED)
		{
			break;
		}

		if(pInputSample == NULL)
		{
			continue;
		}

		const BOOL bVideo = dwReaderStreamIndex == MediaIndex.dwReaderVideoStreamIndex;

		if (bVideo)
		{
			if (FAILED(hr = ProcessStabilization(pTransform, pInputSample, &pOutputSample)))
			{
				break;
			}

			if (pOutputSample != NULL)
			{
				TraceWriteSample(pOutputSample, bVideo);

				LOG_HRESULT(hr = pSinkWriter->WriteSample(dwWriterStreamIndex, pOutputSample));

				SAFE_RELEASE(pOutputSample);
			}
		}
		else
		{
			TraceWriteSample(pInputSample, bVideo);
			LOG_HRESULT(hr = pSinkWriter->WriteSample(dwWriterStreamIndex, pInputSample));
		}

		SAFE_RELEASE(pInputSample);
	}

	SAFE_RELEASE(pOutputSample);
	SAFE_RELEASE(pInputSample);

	IF_FAILED_RETURN(pSinkWriter->Finalize());

	return hr;
}

HRESULT ProcessStabilization(IMFTransform* pTransform, IMFSample* pInputSample, IMFSample** ppOutputSample)
{
	DWORD dwStatus;
	MFT_OUTPUT_DATA_BUFFER outputDataBuffer = {0};

	HRESULT hr = pTransform->ProcessInput(0, pInputSample, 0);

	LOG_HRESULT(hr);

	if (hr != S_OK && hr != MF_E_NOTACCEPTING)
	{
		return hr;
	}

	hr = pTransform->ProcessOutput(0, 1, &outputDataBuffer, &dwStatus);

	assert(outputDataBuffer.pEvents == NULL);

	LOG_HRESULT(hr);

	if(hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
		return S_OK;
	else if(hr != S_OK)
		return hr;

	IF_FAILED_RETURN(pTransform->GetOutputStatus(&dwStatus));
	assert(dwStatus != MFT_OUTPUT_STATUS_SAMPLE_READY);

	*ppOutputSample = outputDataBuffer.pSample;

	return hr;
}

HRESULT CheckNewMediaType(IMFSourceReader* pSourceReader, IMFTransform* /*pTransform*/, IMFSinkWriter* pSinkWriter, const DWORD dwReaderStreamIndex, const DWORD dwWriterStreamIndex)
{
	HRESULT hr = S_OK;
	IMFMediaType* pMediatype = NULL;

	try
	{
		// todo : IMFTransform new media type
		IF_FAILED_THROW(pSourceReader->GetCurrentMediaType(dwReaderStreamIndex, &pMediatype));
		IF_FAILED_THROW(pSinkWriter->SetInputMediaType(dwWriterStreamIndex, pMediatype, NULL));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediatype);

	return hr;
}

void TraceWriteSample(IMFSample* pSample, const BOOL bVideo)
{
	LONGLONG llDuration = -1;
	LONGLONG llTime = -1;
	DWORD dwLenght = 0;

	static int iVideoFrame = 0;
	static int iAudioFrame = 0;

	pSample->GetSampleDuration(&llDuration);
	pSample->GetSampleTime(&llTime);
	pSample->GetTotalLength(&dwLenght);

	// todo : if(bVideo) check MFSampleExtension_VideoDSPMode

	TRACE((L"Encoding %s : frame : %d - size : %ul - duration : %I64d - time : %I64d", bVideo ? L"video" : L"audio", bVideo ? iVideoFrame : iAudioFrame, dwLenght, llDuration, llTime));

	if(bVideo)
		iVideoFrame++;
	else
		iAudioFrame++;
}