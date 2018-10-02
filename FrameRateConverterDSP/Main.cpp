//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "wmcodecdspuuid")

#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <Wmcodecdsp.h>

// Change the URL
#define VIDEO_FILE L"C:\\Users\\Public\\Videos\\Sample Videos\\Wildlife.wmv"

template <class T> inline void SAFE_RELEASE(T*& p){

	if(p){
		p->Release();
		p = NULL;
	}
}

HRESULT ProcessConverter();
HRESULT SetupDMO(IMFTransform*, IMFMediaType*);
HRESULT ProcessSample(IMFSourceReader*, IMFTransform*);
HRESULT ProcessDMO(IMFTransform*, IMFSample*, DWORD&, const UINT32);
HRESULT InitOutputDataBuffer(IMFTransform*, MFT_OUTPUT_DATA_BUFFER*, const UINT32);
HRESULT ChangeCurrentMediaType(IMFSourceReader*, IMFTransform*, UINT32*);

void main(){

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if(SUCCEEDED(hr)){

		hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);

		if(SUCCEEDED(hr)){

			hr = ProcessConverter();

			hr = MFShutdown();
		}

		CoUninitialize();
	}
}

HRESULT ProcessConverter(){

	HRESULT hr;
	IMFSourceReader* pReader = NULL;

	if(FAILED(hr = MFCreateSourceReaderFromURL(VIDEO_FILE, NULL, &pReader))){
		return hr;
	}

	DWORD dwMediaTypeIndex = 0;
	IMFMediaType* pType = NULL;

	hr = pReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, dwMediaTypeIndex, &pType);

	if(SUCCEEDED(hr)){

		UINT32 uiWidth = 0;
		UINT32 uiHeight = 0;

		// We must ask for a subtype compatible with DMO :
		// ARGB32 RGB24 RGB32 RGB555 RGB565 AYUV IYUV UYVY Y211 Y411 Y41P YUY2 YUYV YV12 YVYU
		hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YV12);

		if(SUCCEEDED(hr)){
			hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight);
		}

		// todo : HRESULT MFCalculateImageSize(REFGUID guidSubtype, UINT32 unWidth, UINT32 unHeight, UINT32* pcbImageSize);
		// We set video frame size. Needed when there is h264 format, for example
		if(SUCCEEDED(hr)){
			hr = pType->SetUINT32(MF_MT_SAMPLE_SIZE, (uiWidth * (uiHeight + (uiHeight / 2))));
		}

		if(SUCCEEDED(hr)){
			hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pType);
		}

		// We need this because we use the MediaType to initialize the Transform
		if(SUCCEEDED(hr)){
			SAFE_RELEASE(pType);
			hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType);
		}

		if(SUCCEEDED(hr)){

			IMFTransform* pTransform = NULL;
			hr = CoCreateInstance(CLSID_CFrameRateConvertDmo, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, reinterpret_cast<void**>(&pTransform));

			if(SUCCEEDED(hr)){
				hr = SetupDMO(pTransform, pType);
			}

			if(SUCCEEDED(hr)){

				hr = ProcessSample(pReader, pTransform);

				// Seems not really needed with the DMO
				/*hr = */ pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
				/*hr = */ pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, NULL);
				/*hr = */ pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, NULL);
			}

			SAFE_RELEASE(pTransform);
		}
	}

	SAFE_RELEASE(pType);
	SAFE_RELEASE(pReader);

	return hr;
}

HRESULT SetupDMO(IMFTransform* pTransform, IMFMediaType* pType){

	HRESULT hr = pTransform->SetInputType(0, pType, 0);

	if(SUCCEEDED(hr)){
		// Change the frame rate as needed, here num = 60000 and den = 1001
		hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 60000, 1001);
		//hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 30000, 1001);
		//hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 25, 1);
	}

	if(SUCCEEDED(hr)){
		hr = pTransform->SetOutputType(0, pType, 0);
	}

	if(SUCCEEDED(hr)){
		hr = pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
	}

	if(SUCCEEDED(hr)){
		hr = pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
	}

	return hr;
}

HRESULT ProcessSample(IMFSourceReader* pReader, IMFTransform* pTransform){

	HRESULT hr;
	IMFMediaType* pType = NULL;

	if(FAILED(hr = pTransform->GetOutputCurrentType(0, &pType))){
		return hr;
	}

	// We need the frame size to create the sample buffer.
	UINT32 uiFrameSize = 0;
	hr = pType->GetUINT32(MF_MT_SAMPLE_SIZE, &uiFrameSize);

	SAFE_RELEASE(pType);

	if(FAILED(hr) || uiFrameSize == 0){
		return hr;
	}

	BOOL bProcess = TRUE;
	DWORD streamIndex;
	DWORD flags;
	LONGLONG llTimeStamp;
	IMFSample* pSample = NULL;
	DWORD dwReaderCount = 0;
	DWORD dwDMOCount = 0;

	while(bProcess){

		hr = pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);

		if(SUCCEEDED(hr) && (flags == 0 || flags == MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)){

			if(flags == MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED){

				hr = ChangeCurrentMediaType(pReader, pTransform, &uiFrameSize);

				if(FAILED(hr))
					bProcess = FALSE;
			}

			if(SUCCEEDED(hr)){

				hr = ProcessDMO(pTransform, pSample, dwDMOCount, uiFrameSize);

				// You can check timestamp from the SourceReader
				//hr = pSample->GetSampleDuration(&llTimeStamp);
				//hr = pSample->GetSampleTime(&llTimeStamp);

				SAFE_RELEASE(pSample);
				dwReaderCount++;
			}
		}
		else{

			bProcess = FALSE;
		}
	}

	// Todo : check dwReaderCount and dwDMOCount here.
	// For example with native frame rate = 30000/1001 and dwReaderCount = 901
	// DMO frame rate = 30000/1001 -> dwReaderCount = 901
	// DMO frame rate = 60000/1001 -> dwReaderCount = 1802
	// DMO frame rate = 25/1 -> dwReaderCount = 752

	SAFE_RELEASE(pSample);

	return hr;
}

HRESULT ProcessDMO(IMFTransform* pTransform, IMFSample* pSample, DWORD& dwDMOCount, const UINT32 uiFrameSize){

	HRESULT hr = S_OK;

	MFT_OUTPUT_DATA_BUFFER outputDataBuffer = {0};
	DWORD processOutputStatus = 0;

	hr = pTransform->ProcessInput(0, pSample, 0);

	if(SUCCEEDED(hr)){

		// Todo : we should avoid recreating the buffer...
		hr = InitOutputDataBuffer(pTransform, &outputDataBuffer, uiFrameSize);
	}

	while(hr == S_OK){

		hr = pTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);

		if(hr == MF_E_TRANSFORM_NEED_MORE_INPUT){
			break;
		}

		// You can check new timestamp from the DMO
		/*if(outputDataBuffer.pSample != NULL){

		LONGLONG llTimeStamp = 0;
		hr = outputDataBuffer.pSample->GetSampleTime(&llTimeStamp);
		hr = outputDataBuffer.pSample->GetSampleDuration(&llTimeStamp);
		}*/

		dwDMOCount++;
	}

	if(outputDataBuffer.pSample != NULL){
		SAFE_RELEASE(outputDataBuffer.pSample);
	}

	return hr;
}

HRESULT InitOutputDataBuffer(IMFTransform* pMFTransform, MFT_OUTPUT_DATA_BUFFER* pOutputBuffer, const UINT32 uiFrameSize){

	MFT_OUTPUT_STREAM_INFO outputStreamInfo;
	DWORD outputStreamId = 0;

	ZeroMemory(&outputStreamInfo, sizeof(outputStreamInfo));
	ZeroMemory(pOutputBuffer, sizeof(*pOutputBuffer));

	HRESULT hr = pMFTransform->GetOutputStreamInfo(outputStreamId, &outputStreamInfo);

	if(SUCCEEDED(hr)){

		if((outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) == 0 &&
			(outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES) == 0){

			IMFSample* pOutputSample = NULL;
			IMFMediaBuffer* pMediaBuffer = NULL;

			hr = MFCreateSample(&pOutputSample);

			if(SUCCEEDED(hr)){
				hr = MFCreateMemoryBuffer(uiFrameSize, &pMediaBuffer);
			}

			if(SUCCEEDED(hr)){
				hr = pOutputSample->AddBuffer(pMediaBuffer);
			}

			if(SUCCEEDED(hr)){
				pOutputBuffer->pSample = pOutputSample;
				pOutputBuffer->pSample->AddRef();
			}

			SAFE_RELEASE(pMediaBuffer);
			SAFE_RELEASE(pOutputSample);
		}
	}

	return hr;
}

HRESULT ChangeCurrentMediaType(IMFSourceReader* pReader, IMFTransform* pTransform, UINT32* puiFrameSize){

	IMFMediaType* pType = NULL;

	HRESULT hr = pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType);

	if(SUCCEEDED(hr)){
		hr = pTransform->SetInputType(0, pType, 0);
	}

	if(SUCCEEDED(hr)){
		// Change the frame rate as needed, here num = 60000 and den = 1001
		hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 60000, 1001);
		//hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 30000, 1001);
		//hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 25, 1);
	}

	if(SUCCEEDED(hr)){
		hr = pTransform->SetOutputType(0, pType, 0);
	}

	if(SUCCEEDED(hr)){
		hr = pType->GetUINT32(MF_MT_SAMPLE_SIZE, puiFrameSize);
	}

	SAFE_RELEASE(pType);

	return hr;
}