//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "Shlwapi")

#include <WinSDKVer.h>
#include <new>
#include <Windows.h>
#include <strsafe.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <Shlwapi.h>

template <class T> void SafeRelease(T **ppT){

	if(*ppT){
		(*ppT)->Release();
		*ppT = NULL;
	}
}

void Trace(const WCHAR* sFormatString, ...){

	HRESULT hr = S_OK;
	va_list va;

	const DWORD TRACE_STRING_LEN = 512;

	WCHAR message[TRACE_STRING_LEN];

	va_start(va, sFormatString);
	hr = StringCchVPrintf(message, TRACE_STRING_LEN, sFormatString, va);
	va_end(va);

	if(SUCCEEDED(hr)){

		OutputDebugString(message);
	}
}

// Format constants
const UINT32 VIDEO_WIDTH = 640;
const UINT32 VIDEO_HEIGHT = 480;
const UINT32 VIDEO_FPS = 30;
const UINT64 VIDEO_FRAME_DURATION = 10 * 1000 * 1000 / VIDEO_FPS;
const UINT32 VIDEO_BIT_RATE = 800000;
const GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_WMV3;
const GUID   VIDEO_INPUT_FORMAT = MFVideoFormat_RGB32;
const UINT32 VIDEO_PELS = VIDEO_WIDTH * VIDEO_HEIGHT;
const UINT32 VIDEO_FRAME_COUNT = 20 * VIDEO_FPS;

// Buffer to hold the video frame data.
DWORD g_VideoFrameBuffer[VIDEO_PELS];

#define VIDEO_OUTPUT_FILE L"output.wmv"

HRESULT InitializeSinkWriter(IMFSinkWriter**, DWORD*, IMFSinkWriterCallback*);
HRESULT WriteFrame(IMFSinkWriter*, DWORD, const LONGLONG&);

class CMFSinkWriterCallback : public IMFSinkWriterCallback{

public:

	CMFSinkWriterCallback(HANDLE hFinalizeEvent) : m_nRefCount(1), m_hFinalizeEvent(hFinalizeEvent){}
	virtual ~CMFSinkWriterCallback(){}

	// IMFSinkWriterCallback methods
	STDMETHODIMP OnFinalize(HRESULT hrStatus){

		OutputDebugString(L"CMFSinkWriterCallback::OnFinalize\n");

		if(m_hFinalizeEvent != NULL){
			SetEvent(m_hFinalizeEvent);
		}

		return hrStatus;
	}

	STDMETHODIMP OnMarker(DWORD /*dwStreamIndex*/, LPVOID /*pvContext*/){

		return S_OK;
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv){

		static const QITAB qit[] = {
			QITABENT(CMFSinkWriterCallback, IMFSinkWriterCallback),
		{0}
		};

		return QISearch(this, qit, riid, ppv);
	}

	STDMETHODIMP_(ULONG) AddRef(){

		return InterlockedIncrement(&m_nRefCount);
	}

	STDMETHODIMP_(ULONG) Release(){

		ULONG refCount = InterlockedDecrement(&m_nRefCount);

		if(refCount == 0){
			delete this;
		}

		return refCount;
	}

private:

	volatile long m_nRefCount;
	HANDLE m_hFinalizeEvent;
};

void main(){

	HANDLE hFinalizeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if(hFinalizeEvent == NULL)
		return;

	// Set all pixels to blue
	for(DWORD i = 0; i < VIDEO_PELS; ++i){
		g_VideoFrameBuffer[i] = 0x000000FF;
	}

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if(SUCCEEDED(hr)){

		hr = MFStartup(MF_VERSION);

		if(SUCCEEDED(hr)){

			IMFSinkWriterCallback* pCallBack = NULL;
			IMFSinkWriter *pSinkWriter = NULL;
			DWORD stream = 0;

			pCallBack = new (std::nothrow)CMFSinkWriterCallback(hFinalizeEvent);

			hr = pCallBack ? S_OK : E_POINTER;

			if(SUCCEEDED(hr)){
				hr = InitializeSinkWriter(&pSinkWriter, &stream, pCallBack);
			}

			if(SUCCEEDED(hr)){

				// Send frames to the sink writer.
				LONGLONG rtStart = 0;

				for(DWORD i = 0; i < VIDEO_FRAME_COUNT; ++i){

					hr = WriteFrame(pSinkWriter, stream, rtStart);

					if(FAILED(hr)){
						break;
					}

					rtStart += VIDEO_FRAME_DURATION;

					Trace(L"Copy new frame : %lld (%u)\n", rtStart, i);
				}
			}

			if(SUCCEEDED(hr)){
				hr = pSinkWriter->Finalize();
				WaitForSingleObject(hFinalizeEvent, INFINITE);
			}

			SafeRelease(&pCallBack);
			SafeRelease(&pSinkWriter);
			MFShutdown();
		}

		CoUninitialize();
	}

	CloseHandle(hFinalizeEvent);
}

HRESULT InitializeSinkWriter(IMFSinkWriter** ppWriter, DWORD* pStreamIndex, IMFSinkWriterCallback* pCallBack){

	*ppWriter = NULL;
	*pStreamIndex = NULL;

	IMFAttributes   *pAttributes = NULL;
	IMFSinkWriter   *pSinkWriter = NULL;
	IMFMediaType    *pMediaTypeOut = NULL;
	IMFMediaType    *pMediaTypeIn = NULL;
	DWORD           streamIndex = 0;

	// Create the empty attribute store.
	HRESULT hr = MFCreateAttributes(&pAttributes, 1);

	if(SUCCEEDED(hr)){
		hr = pAttributes->SetUnknown(MF_SINK_WRITER_ASYNC_CALLBACK, pCallBack);
	}

	if(SUCCEEDED(hr)){
		hr = MFCreateSinkWriterFromURL(VIDEO_OUTPUT_FILE, NULL, pAttributes, &pSinkWriter);
	}

	// Set the output media type.
	if(SUCCEEDED(hr)){
		hr = MFCreateMediaType(&pMediaTypeOut);
	}

	if(SUCCEEDED(hr)){
		hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}

	if(SUCCEEDED(hr)){
		hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, VIDEO_ENCODING_FORMAT);
	}

	if(SUCCEEDED(hr)){
		hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, VIDEO_BIT_RATE);
	}

	if(SUCCEEDED(hr)){
		hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}

	if(SUCCEEDED(hr)){
		hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT);
	}

	if(SUCCEEDED(hr)){
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
	}

	if(SUCCEEDED(hr)){
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}

	if(SUCCEEDED(hr)){
		hr = pSinkWriter->AddStream(pMediaTypeOut, &streamIndex);
	}

	// Set the input media type.
	if(SUCCEEDED(hr)){
		hr = MFCreateMediaType(&pMediaTypeIn);
	}

	if(SUCCEEDED(hr)){
		hr = pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}

	if(SUCCEEDED(hr)){
		hr = pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, VIDEO_INPUT_FORMAT);
	}

	if(SUCCEEDED(hr)){
		hr = pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}

	if(SUCCEEDED(hr)){
		hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT);
	}

	if(SUCCEEDED(hr)){
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
	}

	if(SUCCEEDED(hr)){
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}

	if(SUCCEEDED(hr)){
		hr = pSinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, NULL);
	}

	// Tell the sink writer to start accepting data.
	if(SUCCEEDED(hr)){
		hr = pSinkWriter->BeginWriting();
	}

	// Return the pointer to the caller.
	if(SUCCEEDED(hr)){
		*ppWriter = pSinkWriter;
		(*ppWriter)->AddRef();
		*pStreamIndex = streamIndex;
	}

	SafeRelease(&pAttributes);
	SafeRelease(&pSinkWriter);
	SafeRelease(&pMediaTypeOut);
	SafeRelease(&pMediaTypeIn);

	return hr;
}

HRESULT WriteFrame(IMFSinkWriter* pWriter, DWORD streamIndex, const LONGLONG& rtStart){

	IMFSample *pSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;

	const LONG cbWidth = 4 * VIDEO_WIDTH;
	const DWORD cbBuffer = cbWidth * VIDEO_HEIGHT;

	BYTE *pData = NULL;

	// Create a new memory buffer.
	HRESULT hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer);

	// Lock the buffer and copy the video frame to the buffer.
	if(SUCCEEDED(hr)){
		hr = pBuffer->Lock(&pData, NULL, NULL);
	}

	if(SUCCEEDED(hr)){

		hr = MFCopyImage(pData, cbWidth, (BYTE*)g_VideoFrameBuffer, cbWidth, cbWidth, VIDEO_HEIGHT);
	}

	if(pBuffer){
		pBuffer->Unlock();
	}

	// Set the data length of the buffer.
	if(SUCCEEDED(hr)){
		hr = pBuffer->SetCurrentLength(cbBuffer);
	}

	// Create a media sample and add the buffer to the sample.
	if(SUCCEEDED(hr)){
		hr = MFCreateSample(&pSample);
	}

	if(SUCCEEDED(hr)){
		hr = pSample->AddBuffer(pBuffer);
	}

	// Set the time stamp and the duration.
	if(SUCCEEDED(hr)){
		hr = pSample->SetSampleTime(rtStart);
	}

	if(SUCCEEDED(hr)){
		hr = pSample->SetSampleDuration(VIDEO_FRAME_DURATION);
	}

	// Send the sample to the Sink Writer.
	if(SUCCEEDED(hr)){
		hr = pWriter->WriteSample(streamIndex, pSample);
	}

	SafeRelease(&pSample);
	SafeRelease(&pBuffer);

	return hr;
}