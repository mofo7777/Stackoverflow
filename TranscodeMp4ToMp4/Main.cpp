//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

#include <WinSDKVer.h>
#include <new>
#include <windows.h>

#include <mfapi.h>
#include <mfidl.h>
#include <wchar.h>

#define MP4_SOURCE_VIDEO_MEDIA_FILE L"big_buck_bunny_720p_50mb.mp4"
#define MP4_FINAL_VIDEO_MEDIA_FILE L"final.mp4"

HRESULT ProcessConverter(LPCWSTR);
HRESULT ConfigureSource(LPCWSTR, IMFMediaSource**, IMFMediaType**, IMFMediaType**, IMFTopologyNode**, IMFTopologyNode**);
HRESULT CreateMediaSource(LPCWSTR, IMFMediaSource**);
HRESULT ConfigureMediaTypeSource(IMFMediaSource*, IMFPresentationDescriptor*, IMFStreamDescriptor*, IMFMediaType**, IMFMediaType**, IMFTopologyNode**, IMFTopologyNode**);
HRESULT CreateTopologyNodeSink(IMFMediaSink*, IMFTopologyNode**, IMFTopologyNode**, IMFMediaType*, IMFMediaType*);
HRESULT CreateSourceStreamNode(IMFMediaSource*, IMFPresentationDescriptor*, IMFStreamDescriptor*, IMFTopologyNode**);
HRESULT ConfigureSinkNode(IMFMediaTypeHandler*, IMFStreamSink*, IMFTopologyNode**, IMFMediaType*);
HRESULT ConfigureTopologyNode(IMFTopology*, IMFTopologyNode*, IMFTopologyNode*, IMFTopologyNode*, IMFTopologyNode*);
HRESULT RunMediaSession(IMFMediaSession*);

template <class T> inline void SAFE_RELEASE(T*& p){

	if(p){
		p->Release();
		p = NULL;
	}
}

void main() {

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if(SUCCEEDED(hr)) {

		hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);

		if(SUCCEEDED(hr)) {

			hr = ProcessConverter(MP4_SOURCE_VIDEO_MEDIA_FILE);

			hr = MFShutdown();
		}

		CoUninitialize();
	}
}

HRESULT ProcessConverter(LPCWSTR wszVideoFile){

	HRESULT hr = S_OK;
	IMFMediaSource* pSource = NULL;
	IMFMediaType* pVideoMediaType = NULL;
	IMFMediaType* pAudioMediaType = NULL;
	IMFByteStream* pByteStream = NULL;
	IMFMediaSink* pMediaSink = NULL;
	IMFTopology* pTopology = NULL;
	IMFTopologyNode* pVideoSourceNode = NULL;
	IMFTopologyNode* pAudioSourceNode = NULL;
	IMFTopologyNode* pVideoSinkNode = NULL;
	IMFTopologyNode* pAudioSinkNode = NULL;
	IMFMediaSession* pSession = NULL;

	hr = MFCreateFile(MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE, MP4_FINAL_VIDEO_MEDIA_FILE, &pByteStream);

	if(FAILED(hr)){ goto done; }

	hr = ConfigureSource(wszVideoFile, &pSource, &pVideoMediaType, &pAudioMediaType, &pVideoSourceNode, &pAudioSourceNode);

	if(FAILED(hr)){ goto done; }

	hr = MFCreateMPEG4MediaSink(pByteStream, pVideoMediaType, pAudioMediaType, &pMediaSink);

	if(FAILED(hr)){ goto done; }

	hr = CreateTopologyNodeSink(pMediaSink, &pVideoSinkNode, &pAudioSinkNode, pVideoMediaType, pAudioMediaType);

	if(FAILED(hr)){ goto done; }

	hr = MFCreateTopology(&pTopology);

	if(FAILED(hr)){ goto done; }

	hr = ConfigureTopologyNode(pTopology, pVideoSourceNode, pAudioSourceNode, pVideoSinkNode, pAudioSinkNode);

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

	if(pMediaSink){

		hr = pMediaSink->Shutdown();
		SAFE_RELEASE(pMediaSink);
	}

	if(pSource){

		hr = pSource->Shutdown();
		SAFE_RELEASE(pSource);
	}

	if(pSession){

		hr = pSession->Shutdown();
		SAFE_RELEASE(pSession);
	}

	SAFE_RELEASE(pByteStream);
	SAFE_RELEASE(pAudioMediaType);
	SAFE_RELEASE(pVideoMediaType);

	SAFE_RELEASE(pAudioSinkNode);
	SAFE_RELEASE(pVideoSinkNode);
	SAFE_RELEASE(pAudioSourceNode);
	SAFE_RELEASE(pVideoSourceNode);

	SAFE_RELEASE(pTopology);

	return hr;
}

HRESULT ConfigureSource(LPCWSTR wszVideoFile, IMFMediaSource** ppSource, IMFMediaType** ppVideoMediaType, IMFMediaType** ppAudioMediaType, IMFTopologyNode** ppVideoSourceNode, IMFTopologyNode** ppVAudioSourceNode){

	HRESULT hr = S_OK;
	IMFPresentationDescriptor* pPresentationDescriptor = NULL;
	IMFStreamDescriptor* pStreamDescriptor = NULL;
	DWORD dwStreamCount = 0;
	BOOL bSelected = FALSE;

	hr = CreateMediaSource(wszVideoFile, ppSource);

	if(FAILED(hr)){ goto done; }

	hr = (*ppSource)->CreatePresentationDescriptor(&pPresentationDescriptor);

	if(FAILED(hr)){ goto done; }

	hr = pPresentationDescriptor->GetStreamDescriptorCount(&dwStreamCount);

	if(FAILED(hr)){ goto done; }

	for(DWORD dwStream = 0; dwStream < dwStreamCount; dwStream++){

		hr = pPresentationDescriptor->GetStreamDescriptorByIndex(dwStream, &bSelected, &pStreamDescriptor);

		if(FAILED(hr)){
			break;
		}

		if(bSelected){

			hr = ConfigureMediaTypeSource(*ppSource, pPresentationDescriptor, pStreamDescriptor, ppVideoMediaType, ppAudioMediaType, ppVideoSourceNode, ppVAudioSourceNode);
		}

		SAFE_RELEASE(pStreamDescriptor);

		if(FAILED(hr) || ((*ppVideoMediaType) && (*ppAudioMediaType))){
			break;
		}
	}

done:

	SAFE_RELEASE(pStreamDescriptor);
	SAFE_RELEASE(pPresentationDescriptor);

	// We just only if video and audio stream are presents
	if((*ppVideoMediaType) == NULL && (*ppAudioMediaType) == NULL)
		hr = E_FAIL;

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

HRESULT ConfigureMediaTypeSource(IMFMediaSource* pSource, IMFPresentationDescriptor* pPresentationDescriptor, IMFStreamDescriptor* pStreamDescriptor, IMFMediaType** ppVideoMediaType,
				 IMFMediaType** ppAudioMediaType, IMFTopologyNode** ppVideoSourceNode, IMFTopologyNode** ppAudioSourceNode){
	
	HRESULT hr = S_OK;
	IMFMediaTypeHandler* pHandler = NULL;
	IMFMediaType* pMediaType = NULL;
	DWORD dwTypeCount = 0;
	GUID MajorType = GUID_NULL;
	
	hr = pStreamDescriptor->GetMediaTypeHandler(&pHandler);
	
	if(FAILED(hr)){ goto done; }
	
	hr = pHandler->GetMediaTypeCount(&dwTypeCount);
	
	if(FAILED(hr)){ goto done; }
	
	for(DWORD dwType = 0; dwType < dwTypeCount; dwType++){
		
		hr = pHandler->GetMediaTypeByIndex(dwType, &pMediaType);
		
		if(hr == S_OK){
			
			hr = pMediaType->GetMajorType(&MajorType);
			
			if(hr == S_OK){
				
				if(MajorType == MFMediaType_Video && (*ppVideoMediaType) == NULL){
					
					hr = pHandler->SetCurrentMediaType(pMediaType);
					
					if(hr == S_OK){
						
						//LogMediaType(pMediaType);
						
						hr = CreateSourceStreamNode(pSource, pPresentationDescriptor, pStreamDescriptor, ppVideoSourceNode);
						
						if(hr == S_OK){
							*ppVideoMediaType = pMediaType;
							(*ppVideoMediaType)->AddRef();
							break;
						}
					}
				}
				else if(MajorType == MFMediaType_Audio && (*ppAudioMediaType) == NULL){
					
					hr = pHandler->SetCurrentMediaType(pMediaType);
					
					if(hr == S_OK){
						
						//LogMediaType(pMediaType);
						
						hr = CreateSourceStreamNode(pSource, pPresentationDescriptor, pStreamDescriptor, ppAudioSourceNode);
						
						if(hr == S_OK){
							*ppAudioMediaType = pMediaType;
							(*ppAudioMediaType)->AddRef();
							break;
						}
					}
				}
			}
		}
		
		SAFE_RELEASE(pMediaType);
	}
	
done:
	
	SAFE_RELEASE(pMediaType);
	SAFE_RELEASE(pHandler);
	
	return hr;
}

HRESULT CreateSourceStreamNode(IMFMediaSource* pSource, IMFPresentationDescriptor* pPresentationDescriptor, IMFStreamDescriptor* pStreamDescriptor, IMFTopologyNode** ppNode){

	HRESULT hr = S_OK;
	IMFTopologyNode* pNode = NULL;
	
	hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
	
	if(FAILED(hr)){ goto done; }
	
	hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
	
	if(FAILED(hr)){ goto done; }
	
	hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPresentationDescriptor);
	
	if(FAILED(hr)){ goto done; }
	
	hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pStreamDescriptor);
	
	if(FAILED(hr)){ goto done; }
	
	*ppNode = pNode;
	(*ppNode)->AddRef();

done:
	
	SAFE_RELEASE(pNode);
	
	return hr;
}

HRESULT CreateTopologyNodeSink(IMFMediaSink* pMediaSink, IMFTopologyNode** ppVideoSinkNode, IMFTopologyNode** ppAudioSinkNode, IMFMediaType* pVideoMediaType, IMFMediaType* pAudioMediaType){

	HRESULT hr = S_OK;
	DWORD dwCount = 0;
	IMFStreamSink* pStreamSink = NULL;
	IMFMediaTypeHandler* pHandler = NULL;
	GUID MajorType = GUID_NULL;
	
	hr = pMediaSink->GetStreamSinkCount(&dwCount);
	
	for(DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++){
		
		hr = pMediaSink->GetStreamSinkByIndex(dwIndex, &pStreamSink);
		
		if(hr == S_OK){
			
			hr = pStreamSink->GetMediaTypeHandler(&pHandler);
			
			if(hr == S_OK){
				
				hr = pHandler->GetMajorType(&MajorType);
				
				if(hr == S_OK){
					
					if(MajorType == MFMediaType_Video)
						hr = ConfigureSinkNode(pHandler, pStreamSink, ppVideoSinkNode, pVideoMediaType);
					else if(MajorType == MFMediaType_Audio)
						hr = ConfigureSinkNode(pHandler, pStreamSink, ppAudioSinkNode, pAudioMediaType);
				}
				
				if(hr == S_OK && (*ppVideoSinkNode) != NULL && (*ppAudioSinkNode) != NULL){
					break;
				}
			}
			
			SAFE_RELEASE(pHandler);
		}
		
		SAFE_RELEASE(pStreamSink);
	}
	
	if(FAILED(hr)){ goto done; }

done:
	
	SAFE_RELEASE(pHandler);
	SAFE_RELEASE(pStreamSink);
	
	if((*ppVideoSinkNode) == NULL || (*ppAudioSinkNode) == NULL)
		hr = E_FAIL;
	
	return hr;
}

HRESULT ConfigureSinkNode(IMFMediaTypeHandler* pHandler, IMFStreamSink* pStreamSink, IMFTopologyNode** ppSinkNode, IMFMediaType* pMediaType){

	HRESULT hr = S_OK;
	IMFTopologyNode* pNode = NULL;
	
	hr = pHandler->SetCurrentMediaType(pMediaType);
	
	if(FAILED(hr)){ goto done; }
	
	hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
	
	if(FAILED(hr)){ goto done; }
	
	hr = pNode->SetObject(pStreamSink);
	
	if(FAILED(hr)){ goto done; }
	
	*ppSinkNode = pNode;
	(*ppSinkNode)->AddRef();

done:
	
	SAFE_RELEASE(pNode);
	
	return hr;
}

HRESULT ConfigureTopologyNode(IMFTopology* pTopology, IMFTopologyNode* pVideoSourceNode, IMFTopologyNode* pAudioSourceNode, IMFTopologyNode* pVideoSinkNode, IMFTopologyNode* pAudioSinkNode){

	HRESULT hr = S_OK;
	
	hr = pTopology->AddNode(pVideoSourceNode);
	
	if(FAILED(hr)){ goto done; }
	
	hr = pTopology->AddNode(pAudioSourceNode);

	if(FAILED(hr)){ goto done; }

	hr = pTopology->AddNode(pVideoSinkNode);

	if(FAILED(hr)){ goto done; }

	hr = pTopology->AddNode(pAudioSinkNode);

	if(FAILED(hr)){ goto done; }

	hr = pVideoSourceNode->ConnectOutput(0, pVideoSinkNode, 0);

	if(FAILED(hr)){ goto done; }

	hr = pAudioSourceNode->ConnectOutput(0, pAudioSinkNode, 0);

done:

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
					wprintf(L"MESessionTopologySet\n");
					break;
					
				case MESessionTopologyStatus:
					
					hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus);
					
					if(SUCCEEDED(hr)){
						
						switch(TopoStatus){
								
							case MF_TOPOSTATUS_READY:
								{
									wprintf(L"MESessionTopologyStatus: MF_TOPOSTATUS_READY\n");
									PROPVARIANT varStartPosition;
									PropVariantInit(&varStartPosition);
									hr = pSession->Start(&GUID_NULL, &varStartPosition);
									PropVariantClear(&varStartPosition);
								}
								break;
								
							case MF_TOPOSTATUS_STARTED_SOURCE:
								wprintf(L"MESessionTopologyStatus: MF_TOPOSTATUS_STARTED_SOURCE\n");
								break;
								
							case MF_TOPOSTATUS_ENDED:
								wprintf(L"MESessionTopologyStatus: MF_TOPOSTATUS_ENDED\n");
								break;
								
							default:
								wprintf(L"MESessionTopologyStatus: %d\n", TopoStatus);
								break;
						}
					}
					break;
					
				case MESessionStarted:
					wprintf(L"MESessionStarted\n");
					break;
					
				case MESessionEnded:
					wprintf(L"MESessionEnded\n");
					hr = pSession->Stop();
					break;
					
				case MESessionStopped:
					wprintf(L"MESessionStopped\n");
					hr = pSession->Close();
					break;
					
				case MESessionClosed:
					wprintf(L"MESessionClosed\n");
					bSessionEvent = FALSE;
					break;
					
				case MESessionNotifyPresentationTime:
					wprintf(L"MESessionNotifyPresentationTime\n");
					break;
					
				case MESessionCapabilitiesChanged:
					wprintf(L"MESessionCapabilitiesChanged\n");
					break;
					
				case MEEndOfPresentation:
					wprintf(L"MEEndOfPresentation\n");
					break;
					
				default:
					wprintf(L"Media session event: %d\n", meType);
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
