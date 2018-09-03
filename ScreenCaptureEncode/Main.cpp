//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "d3d9.lib")

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <d3d9.h>

template <class T> void SafeRelease(T **ppT){

	if(*ppT){
		(*ppT)->Release();
		*ppT = NULL;
	}
}

#define REVERSE_IMAGE

// Format constants
const UINT32 VIDEO_FPS = 30;
const UINT64 VIDEO_FRAME_DURATION = 10 * 1000 * 1000 / VIDEO_FPS;
const UINT32 VIDEO_BIT_RATE = 2000000;
const GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_H264;
const GUID   VIDEO_INPUT_FORMAT = MFVideoFormat_RGB32;
const UINT32 VIDEO_FRAME_COUNT = 5 * VIDEO_FPS;

HRESULT InitializeDirect3D9(IDirect3DDevice9** ppDevice, IDirect3DSurface9** ppSurface, UINT32& uiWidth, UINT32& uiHeight){

	IDirect3D9* d3d = NULL;

	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if(d3d == NULL)
		return E_POINTER;
	
	D3DDISPLAYMODE mode;
	HRESULT hr = d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);

	if(FAILED(hr)){
		SafeRelease(&d3d);
		return hr;
	}

	D3DPRESENT_PARAMETERS parameters = {0};

	parameters.Windowed = TRUE;
	parameters.BackBufferCount = 1;
	uiHeight = parameters.BackBufferHeight = mode.Height;
	uiWidth = parameters.BackBufferWidth = mode.Width;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = NULL;

	hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, ppDevice);

	if(FAILED(hr)){
		SafeRelease(&d3d);
		return hr;
	}

	hr = (*ppDevice)->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, ppSurface, nullptr);

	SafeRelease(&d3d);

	return hr;
}

HRESULT InitializeSinkWriter(IMFSinkWriter **ppWriter, DWORD *pStreamIndex, const UINT32 uiWidth, const UINT32 uiHeight){

	*ppWriter = NULL;
	*pStreamIndex = NULL;

	IMFSinkWriter   *pSinkWriter = NULL;
	IMFMediaType    *pMediaTypeOut = NULL;
	IMFMediaType    *pMediaTypeIn = NULL;
	DWORD           streamIndex;

	HRESULT hr = MFCreateSinkWriterFromURL(L"output.mp4", NULL, NULL, &pSinkWriter);

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
		hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, uiWidth, uiHeight);
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
		hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, uiWidth, uiHeight);
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

	SafeRelease(&pSinkWriter);
	SafeRelease(&pMediaTypeOut);
	SafeRelease(&pMediaTypeIn);
	return hr;
}

HRESULT WriteFrame(IDirect3DDevice9* pDevice, IDirect3DSurface9* pSurface, IMFSinkWriter* pWriter, DWORD streamIndex, const LONGLONG& rtStart, const UINT32 uiWidth, const UINT32 uiHeight){

	HRESULT hr = pDevice->GetFrontBufferData(0, pSurface);

	if(FAILED(hr)){
		return hr;
	}

	D3DLOCKED_RECT rc;
	hr = pSurface->LockRect(&rc, NULL, 0);

	if(FAILED(hr)){
		return hr;
	}

	IMFSample *pSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;

	const LONG cbWidth = 4 * uiWidth;
	const DWORD cbBuffer = cbWidth * uiHeight;

	BYTE *pData = NULL;

	// Create a new memory buffer.
	hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer);

	// Lock the buffer and copy the video frame to the buffer.
	if(SUCCEEDED(hr)){
		hr = pBuffer->Lock(&pData, NULL, NULL);
	}

	if(SUCCEEDED(hr)){

#ifdef REVERSE_IMAGE
		for(int i = 0, j = uiHeight - 1; i < uiHeight; i++, j--)
			for(int k = 0; k < cbWidth; k++)
				pData[(i * cbWidth) + k] = ((BYTE*)rc.pBits)[(j * cbWidth) + k];
#else
		hr = MFCopyImage(pData, cbWidth, (BYTE*)rc.pBits, rc.Pitch, cbWidth, uiHeight);
#endif
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

	hr = pSurface->UnlockRect();

	SafeRelease(&pSample);
	SafeRelease(&pBuffer);
	return hr;
}

void main(){

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if(SUCCEEDED(hr)){

		hr = MFStartup(MF_VERSION);

		if(SUCCEEDED(hr)){

			UINT32 uiWidth = 0;
			UINT32 uiHeight = 0;

			IDirect3DDevice9* pDevice = NULL;
			IDirect3DSurface9* pSurface = NULL;

			hr = InitializeDirect3D9(&pDevice, &pSurface, uiWidth, uiHeight);

			if(SUCCEEDED(hr)){

				IMFSinkWriter *pSinkWriter = NULL;
				DWORD stream;

				hr = InitializeSinkWriter(&pSinkWriter, &stream, uiWidth, uiHeight);

				if(SUCCEEDED(hr)){

					LONGLONG rtStart = 0;

					for(DWORD i = 0; i < VIDEO_FRAME_COUNT; ++i){

						hr = WriteFrame(pDevice, pSurface, pSinkWriter, stream, rtStart, uiWidth, uiHeight);

						if(FAILED(hr)){
							break;
						}

						rtStart += VIDEO_FRAME_DURATION;
					}
				}

				if(SUCCEEDED(hr)){
					hr = pSinkWriter->Finalize();
				}

				SafeRelease(&pSinkWriter);
			}

			SafeRelease(&pDevice);
			SafeRelease(&pSurface);
			MFShutdown();
		}

		CoUninitialize();
	}
}