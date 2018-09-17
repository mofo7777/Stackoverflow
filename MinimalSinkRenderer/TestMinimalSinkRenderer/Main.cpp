//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <strsafe.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <initguid.h>
#include <Shlwapi.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <d3d9.h>
#include <Evr9.h>
#include <ks.h>

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
#define MF_USE_LOGGING 1
#else
#define MF_USE_LOGGING 0
#endif

#include "..\Common\MFMacro.h"
#include "..\Common\MFTrace.h"
#include "..\Common\MFLogging.h"
#include "..\Common\MFTExternTrace.h"
#include "..\Common\MFGuid.h"

#define MP4_SOURCE_VIDEO_FILE L"big_buck_bunny_720p_50mb.mp4"

HRESULT ProcessMediaSession(LPCWSTR);
HRESULT CreateMediaSource(LPCWSTR, IMFMediaSource**);
HRESULT CreateTopology(IMFTopology**, IMFMediaSink**, IMFMediaSource*);
HRESULT BuildTopology(IMFMediaSink**, IMFTopology*, IMFPresentationDescriptor*, IMFMediaSource*);
HRESULT CreateSourceStreamNode(IMFMediaSource*, IMFPresentationDescriptor*, IMFStreamDescriptor*, IMFTopologyNode**);
HRESULT CreateOutputNode(IMFStreamDescriptor*, IMFMediaSink**, IMFTopologyNode**);
HRESULT RunMediaSession(IMFMediaSession*);

void main() {

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if(SUCCEEDED(hr)) {

		hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);

		if(SUCCEEDED(hr)) {

			hr = ProcessMediaSession(MP4_SOURCE_VIDEO_FILE);

			hr = MFShutdown();
		}

		CoUninitialize();
	}
}

HRESULT ProcessMediaSession(LPCWSTR wszVideoFile){

	HRESULT hr = S_OK;

	IMFMediaSource* pSource = NULL;
	IMFTopology* pTopology = NULL;
	IMFMediaSink* pMinimalSink = NULL;
	IMFMediaSession* pSession = NULL;

	hr = CreateMediaSource(wszVideoFile, &pSource);
	if(FAILED(hr)){ goto done; }

	hr = CreateTopology(&pTopology, &pMinimalSink, pSource);
	if(FAILED(hr)){ goto done; }

	hr = MFCreateMediaSession(NULL, &pSession);
	if(FAILED(hr)){ goto done; }

	hr = pSession->SetTopology(0, pTopology);
	if(FAILED(hr)){ goto done; }

	hr = RunMediaSession(pSession);

done:

	if(pSession){

		hr = pSession->Close();

		// todo : normally wait for close event, here just Sleep
		Sleep(1000);
	}

	if(pMinimalSink){

		hr = pMinimalSink->Shutdown();
		SAFE_RELEASE(pMinimalSink);
	}

	if(pSource){

		hr = pSource->Shutdown();
		SAFE_RELEASE(pSource);
	}

	if(pSession){

		hr = pSession->Shutdown();
		SAFE_RELEASE(pSession);
	}

	SAFE_RELEASE(pTopology);

	return hr;
}

HRESULT CreateMediaSource(LPCWSTR wszVideoFile, IMFMediaSource** ppSource){

	MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

	IMFSourceResolver* pSourceResolver = NULL;
	IUnknown* pSource = NULL;

	HRESULT hr = MFCreateSourceResolver(&pSourceResolver);

	if(FAILED(hr)){ goto done; }

	hr = pSourceResolver->CreateObjectFromURL(wszVideoFile, MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, &pSource);

	if(FAILED(hr)){ goto done; }

	hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource));

done:

	SAFE_RELEASE(pSourceResolver);
	SAFE_RELEASE(pSource);

	return hr;
}

HRESULT CreateTopology(IMFTopology** ppTopology, IMFMediaSink** ppMinimalSink, IMFMediaSource* pSource){

	HRESULT hr = S_OK;

	IMFTopology* pTopology = NULL;
	IMFPresentationDescriptor* pSourcePD = NULL;

	hr = MFCreateTopology(&pTopology);
	if(FAILED(hr)){ goto done; }

	hr = pSource->CreatePresentationDescriptor(&pSourcePD);
	if(FAILED(hr)){ goto done; }

	hr = BuildTopology(ppMinimalSink, pTopology, pSourcePD, pSource);
	if(FAILED(hr)){ goto done; }

	*ppTopology = pTopology;
	(*ppTopology)->AddRef();

done:

	SAFE_RELEASE(pTopology);
	SAFE_RELEASE(pSourcePD);

	return hr;
}

HRESULT BuildTopology(IMFMediaSink** ppMinimalSink, IMFTopology* pTopology, IMFPresentationDescriptor* pSourcePD, IMFMediaSource* pSource){

	HRESULT hr = S_OK;

	IMFStreamDescriptor* pSourceSD = NULL;
	IMFTopologyNode* pSourceNode = NULL;
	IMFTopologyNode* pOutputNode = NULL;
	BOOL bSelected = FALSE;
	DWORD dwStreamCount;

	hr = pSourcePD->GetStreamDescriptorCount(&dwStreamCount);
	if(FAILED(hr)){ goto done; }

	for(DWORD i = 0; i < dwStreamCount; i++){

		hr = pSourcePD->GetStreamDescriptorByIndex(i, &bSelected, &pSourceSD);
		if(FAILED(hr)){ goto done; }

		if(bSelected){

			hr = CreateSourceStreamNode(pSource, pSourcePD, pSourceSD, &pSourceNode);
			if(FAILED(hr)){ goto done; }

			hr = CreateOutputNode(pSourceSD, ppMinimalSink, &pOutputNode);
			if(FAILED(hr)){ goto done; }

			hr = pTopology->AddNode(pSourceNode);
			if(FAILED(hr)){ goto done; }

			hr = pTopology->AddNode(pOutputNode);
			if(FAILED(hr)){ goto done; }

			hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);
			if(FAILED(hr)){ goto done; }

			SAFE_RELEASE(pOutputNode);
			SAFE_RELEASE(pSourceNode);
		}

		SAFE_RELEASE(pSourceSD);
	}

done:

	SAFE_RELEASE(pOutputNode);
	SAFE_RELEASE(pSourceNode);
	SAFE_RELEASE(pSourceSD);

	return hr;
}

HRESULT CreateSourceStreamNode(IMFMediaSource* pSource, IMFPresentationDescriptor* pSourcePD, IMFStreamDescriptor* pSourceSD, IMFTopologyNode** ppNode){

	HRESULT hr = S_OK;

	IMFTopologyNode* pNode = NULL;

	hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
	if(FAILED(hr)){ goto done; }

	hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
	if(FAILED(hr)){ goto done; }

	hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pSourcePD);
	if(FAILED(hr)){ goto done; }

	hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSourceSD);
	if(FAILED(hr)){ goto done; }

	*ppNode = pNode;
	(*ppNode)->AddRef();

done:

	SAFE_RELEASE(pNode);

	return hr;
}

HRESULT CreateOutputNode(IMFStreamDescriptor* pSourceSD, IMFMediaSink** ppMinimalSink, IMFTopologyNode** ppNode){

	HRESULT hr = S_OK; 
	
	IMFTopologyNode* pNode = NULL;
	IMFMediaTypeHandler* pHandler = NULL;
	IMFActivate* pActivate = NULL;
	IMFStreamSink* pStreamSink = NULL;
	GUID guidMajorType = GUID_NULL;
	DWORD dwStreamSinkCount;

	hr = pSourceSD->GetMediaTypeHandler(&pHandler);
	if(FAILED(hr)){ goto done; }

	hr = pHandler->GetMajorType(&guidMajorType);
	if(FAILED(hr)){ goto done; }

	hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
	if(FAILED(hr)){ goto done; }

	if(MFMediaType_Video == guidMajorType){

		hr = CoCreateInstance(CLSID_MinimalSinkRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IMFMediaSink, reinterpret_cast<void**>(ppMinimalSink));
		if(FAILED(hr)){ goto done; }

		hr = (*ppMinimalSink)->GetStreamSinkCount(&dwStreamSinkCount);
		if(FAILED(hr)){ goto done; }

		hr = (*ppMinimalSink)->GetStreamSinkByIndex(0, &pStreamSink);
		if(FAILED(hr)){ goto done; }

		hr = pNode->SetObject(pStreamSink);
		if(FAILED(hr)){ goto done; }
	}
	else if(MFMediaType_Audio == guidMajorType){

		hr = MFCreateAudioRendererActivate(&pActivate);
		if(FAILED(hr)){ goto done; }

		hr = pNode->SetObject(pActivate);
		if(FAILED(hr)){ goto done; }
	}
	else{

		hr = E_FAIL;
		if(FAILED(hr)){ goto done; }
	}

	*ppNode = pNode;
	(*ppNode)->AddRef();

done:

	SAFE_RELEASE(pStreamSink);
	SAFE_RELEASE(pActivate);
	SAFE_RELEASE(pHandler);
	SAFE_RELEASE(pNode);

	return hr;
}

HRESULT RunMediaSession(IMFMediaSession* pSession){

	HRESULT hr = S_OK;

	BOOL bSessionEvent = TRUE;

	while(bSessionEvent){

		HRESULT hrStatus = S_OK;
		IMFMediaEvent* pEvent = NULL;
		MediaEventType meType = MEUnknown;

		MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID;

		hr = pSession->GetEvent(0, &pEvent);

		if(SUCCEEDED(hr)){
			hr = pEvent->GetStatus(&hrStatus);
		}

		if(SUCCEEDED(hr)){
			hr = pEvent->GetType(&meType);
		}

		if(SUCCEEDED(hr) && SUCCEEDED(hrStatus)){

			switch(meType){

			case MESessionTopologySet:
				TRACE((L"MESessionTopologySet"));
				break;

			case MESessionTopologyStatus:

				hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus);

				if(SUCCEEDED(hr)){

					switch(TopoStatus){

					case MF_TOPOSTATUS_READY:
					{
						TRACE((L"MESessionTopologyStatus: MF_TOPOSTATUS_READY"));
						PROPVARIANT varStartPosition;
						PropVariantInit(&varStartPosition);
						hr = pSession->Start(&GUID_NULL, &varStartPosition);
						PropVariantClear(&varStartPosition);
					}
					break;

					case MF_TOPOSTATUS_STARTED_SOURCE:
						TRACE((L"MESessionTopologyStatus: MF_TOPOSTATUS_STARTED_SOURCE"));
						break;

					case MF_TOPOSTATUS_ENDED:
						TRACE((L"MESessionTopologyStatus: MF_TOPOSTATUS_ENDED"));
						break;

					default:
						TRACE((L"MESessionTopologyStatus: %d", TopoStatus));
						break;
					}
				}
				break;

			case MESessionStarted:
				TRACE((L"MESessionStarted"));
				break;

			case MESessionEnded:
				TRACE((L"MESessionEnded"));
				hr = pSession->Stop();
				break;

			case MESessionStopped:
				TRACE((L"MESessionStopped"));
				hr = pSession->Close();
				break;

			case MESessionClosed:
				TRACE((L"MESessionClosed"));
				bSessionEvent = FALSE;
				break;

			case MESessionNotifyPresentationTime:
				TRACE((L"MESessionNotifyPresentationTime"));
				break;

			case MESessionCapabilitiesChanged:
				TRACE((L"MESessionCapabilitiesChanged"));
				break;

			case MEEndOfPresentation:
				TRACE((L"MEEndOfPresentation"));
				break;

			default:
				TRACE((L"Media session event: %d", meType));
				break;
			}

			SAFE_RELEASE(pEvent);

			if(FAILED(hr) || FAILED(hrStatus)){
				bSessionEvent = FALSE;
			}
		}
	}

	return hr;
}