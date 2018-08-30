//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "shlwapi")

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
#include <ks.h>

//----------------------------------------------------------------------------------------------
// Microsoft DirectX SDK (June 2010)
#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>
#include <Evr9.h>

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
#include "..\Common\MFCriticSection.h"

#define WINDOWAPPLICATION_CLASS L"WindowApplication"

//------------------------------------------------------------------------------------
// Use same file (make a copy), then set video width and height according to your file
#define VIDEO_WIDTH_1 320
#define VIDEO_HEIGHT_1 240
#define VIDEO_FILE_1 L"big_buck_bunny_240p_5mb.mp4"
#define VIDEO_FILE_2 L"big_buck_bunny_240p_5mb - Copie.mp4"
//------------------------------------------------------------------------------------

HWND g_hWnd = NULL;
HANDLE g_hSessionEvent = NULL;
IMFMediaSession* g_pSession = NULL;
IMFMediaSource* g_pVideoSource1 = NULL;
IMFMediaSource* g_pVideoSource2 = NULL;
IMFMediaSource* g_pAggregatedSource = NULL;

class CCustomAsyncCallback : public IMFAsyncCallback {

public:

		CCustomAsyncCallback() : m_nRefCount(1) {}
		virtual ~CCustomAsyncCallback() {}

		// IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {

				static const QITAB qit[] = {
						QITABENT(CCustomAsyncCallback, IMFAsyncCallback),
				{0}
				};

				return QISearch(this, qit, riid, ppv);
		}
		STDMETHODIMP_(ULONG) AddRef() {

				LONG lRef = InterlockedIncrement(&m_nRefCount);
				return lRef;
		}
		STDMETHODIMP_(ULONG) Release() {

				ULONG uCount = InterlockedDecrement(&m_nRefCount);

				if(uCount == 0) {
						delete this;
				}

				return uCount;
		}

		// IMFAsyncCallback
		STDMETHODIMP GetParameters(DWORD*, DWORD*) { return E_NOTIMPL; }
		STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) {

				IMFMediaEvent* pEvent = NULL;
				HRESULT hr = S_OK;
				HRESULT hrStatus;
				MediaEventType EventType;

				AutoLock lock(m_CriticSection);

				try {

						IF_FAILED_THROW(hr = g_pSession->EndGetEvent(pAsyncResult, &pEvent));

						IF_FAILED_THROW(hr = pEvent->GetType(&EventType));

						TRACE((L"Invoke %s", MFEventString(EventType)));

						IF_FAILED_THROW(hr = pEvent->GetStatus(&hrStatus));

						if(FAILED(hrStatus)) {

								LOG_HRESULT(hr = hrStatus);

								LOG_HRESULT(hr = g_pSession->BeginGetEvent(this, NULL));
								SAFE_RELEASE(pEvent);

								//SetEvent(g_hSessionEvent);

								return S_OK;
						}

						if(EventType == MESessionTopologyStatus) {

								MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID;
								LOG_HRESULT(hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));
								TRACE((L"TopoStatus %s", MFTopologyStatusString(TopoStatus)));

								if(TopoStatus == MF_TOPOSTATUS_READY)
										SetEvent(g_hSessionEvent);
						}

						if(EventType != MESessionClosed) {

								LOG_HRESULT(hr = g_pSession->BeginGetEvent(this, NULL));
						}
						else {
								SetEvent(g_hSessionEvent);
						}
				}
				catch(HRESULT) {}

				SAFE_RELEASE(pEvent);

				return S_OK;
		}

private:

		CriticSection m_CriticSection;
		volatile long m_nRefCount;
};

CCustomAsyncCallback* g_pCustomAsyncCallback = NULL;

void FreeMediaObject();
HRESULT ProcessVideo();
HRESULT CreateMediaSource(IMFMediaSource**, LPCWSTR);
HRESULT CreateAggregatedSource(IMFMediaSource*, IMFMediaSource*, IMFMediaSource**);
HRESULT CreateTopologyAggregated(IMFTopology**, IMFMediaSource*);
HRESULT BuildTopology(IMFTopology*, IMFPresentationDescriptor*, IMFMediaSource*, IMFStreamSink*, IMFStreamSink*);
HRESULT CreateSourceStreamNode(IMFMediaSource*, IMFPresentationDescriptor*, IMFStreamDescriptor*, IMFTopologyNode**);
HRESULT CreateOutputNode(IMFStreamDescriptor*, IMFTopologyNode**, IMFStreamSink*);
HRESULT InitWindow(const UINT, const UINT);
LRESULT CALLBACK WindowApplicationMsgProc(HWND, UINT, WPARAM, LPARAM);

void main() {

		HRESULT hr;

		LOG_HRESULT(hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));

		if(SUCCEEDED(hr)) {

				LOG_HRESULT(hr = MFStartup(MF_VERSION, MFSTARTUP_LITE));

				if(SUCCEEDED(hr)) {

						LOG_HRESULT(hr = ProcessVideo());

						if(SUCCEEDED(hr)) {

								MSG msg;
								ZeroMemory(&msg, sizeof(MSG));

								while(GetMessage(&msg, NULL, 0, 0) > 0) {

										TranslateMessage(&msg);
										DispatchMessage(&msg);
								}
						}

						FreeMediaObject();

						LOG_HRESULT(hr = MFShutdown());
				}

				CoUninitialize();
		}
}

void FreeMediaObject() {

		HRESULT hr = S_OK;

		if(g_pSession != NULL) {

				LOG_HRESULT(hr = g_pSession->Close());

				DWORD dwWaitResult = WaitForSingleObject(g_hSessionEvent, 10000);

				if(dwWaitResult == WAIT_TIMEOUT)
				{
						assert(FALSE);
				}
		}

		if(g_pAggregatedSource) {

				g_pAggregatedSource->Shutdown();
				SAFE_RELEASE(g_pAggregatedSource);
		}

		SAFE_RELEASE(g_pVideoSource1);
		SAFE_RELEASE(g_pVideoSource2);
		SAFE_RELEASE(g_pCustomAsyncCallback);

		if(g_pSession) {

				LOG_HRESULT(hr = g_pSession->Shutdown());

				ULONG ulTest = g_pSession->Release();
				g_pSession = NULL;

				assert(ulTest == 0);
		}

		if(g_hSessionEvent)
		{
				CloseHandle(g_hSessionEvent);
				g_hSessionEvent = NULL;
		}

		if(IsWindow(g_hWnd)) {

				DestroyWindow(g_hWnd);
				UnregisterClass(WINDOWAPPLICATION_CLASS, GetModuleHandle(NULL));
				g_hWnd = NULL;
		}
}

HRESULT ProcessVideo() {

		HRESULT hr = S_OK;
		IMFTopology* pTopology = NULL;

		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_EMPTY;

		try {

				g_pCustomAsyncCallback = new (std::nothrow)CCustomAsyncCallback();
				IF_FAILED_THROW(hr = (g_pCustomAsyncCallback == NULL ? E_OUTOFMEMORY : S_OK));

				g_hSessionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
				IF_FAILED_THROW(hr = (g_hSessionEvent == NULL ? E_OUTOFMEMORY : S_OK));

				IF_FAILED_THROW(hr = CreateMediaSource(&g_pVideoSource1, VIDEO_FILE_1));
				IF_FAILED_THROW(hr = CreateMediaSource(&g_pVideoSource2, VIDEO_FILE_2));
				IF_FAILED_THROW(hr = CreateAggregatedSource(g_pVideoSource1, g_pVideoSource2, &g_pAggregatedSource));

				IF_FAILED_THROW(hr = CreateTopologyAggregated(&pTopology, g_pAggregatedSource));
				IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &g_pSession));
				IF_FAILED_THROW(hr = g_pSession->BeginGetEvent((IMFAsyncCallback*)g_pCustomAsyncCallback, NULL));
				IF_FAILED_THROW(hr = g_pSession->SetTopology(0, pTopology));

				DWORD dwWaitResult = WaitForSingleObject(g_hSessionEvent, 10000);

				if(dwWaitResult == WAIT_TIMEOUT)
				{
						IF_FAILED_THROW(hr = E_FAIL);
				}

				LOG_HRESULT(hr = g_pSession->Start(&GUID_NULL, &varStart));
		}
		catch(HRESULT) {}

		SAFE_RELEASE(pTopology);

		PropVariantClear(&varStart);

		return hr;
}

HRESULT CreateMediaSource(IMFMediaSource** ppSource, LPCWSTR szURL) {

		HRESULT hr = S_OK;
		MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

		IMFSourceResolver* pSourceResolver = NULL;
		IUnknown* pSource = NULL;

		try {

				IF_FAILED_THROW(hr = MFCreateSourceResolver(&pSourceResolver));
				IF_FAILED_THROW(hr = pSourceResolver->CreateObjectFromURL(szURL, MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, &pSource));
				IF_FAILED_THROW(hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource)));
		}
		catch(HRESULT) {}

		SAFE_RELEASE(pSource);
		SAFE_RELEASE(pSourceResolver);

		return hr;
}

HRESULT CreateAggregatedSource(IMFMediaSource* pSource1, IMFMediaSource* pSource2, IMFMediaSource** ppAggregatedSource) {

		IMFCollection* pCollection = NULL;

		HRESULT hr = MFCreateCollection(&pCollection);

		if(SUCCEEDED(hr)) {

				hr = pCollection->AddElement(pSource1);
		}

		if(SUCCEEDED(hr)) {

				hr = pCollection->AddElement(pSource2);
		}

		if(SUCCEEDED(hr)) {

				hr = MFCreateAggregateSource(pCollection, ppAggregatedSource);
		}

		SAFE_RELEASE(pCollection);

		return hr;
}

HRESULT CreateTopologyAggregated(IMFTopology** ppTopology, IMFMediaSource* pSource) {

		assert(ppTopology != NULL);
		assert(pSource != NULL);

		HRESULT hr = S_OK;

		IMFTopology* pTopology = NULL;
		IMFPresentationDescriptor* pSourcePD = NULL;
		IMFActivate* pEvrActivate = NULL;
		//IMFVideoRenderer* pVideoRenderer = NULL;
		IMFMediaSink* pEvrSink = NULL;
		IMFStreamSink* pStreamSink1 = NULL;
		IMFStreamSink* pStreamSink2 = NULL;

		try {

				IF_FAILED_THROW(hr = MFCreateTopology(&pTopology));

				IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(&pSourcePD));

				IF_FAILED_THROW(hr = InitWindow(VIDEO_WIDTH_1, VIDEO_HEIGHT_1));
				IF_FAILED_THROW(hr = MFCreateVideoRendererActivate(g_hWnd, &pEvrActivate));
				IF_FAILED_THROW(hr = pEvrActivate->SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_MIXER_CLSID, CLSID_CustomVideoMixer));
				//IF_FAILED_THROW(hr = pEvrActivate->ActivateObject(__uuidof(IMFVideoRenderer), reinterpret_cast<void**>(&pVideoRenderer)));
				//IF_FAILED_THROW(hr = pVideoRenderer->InitializeRenderer(NULL, NULL));

				IF_FAILED_THROW(hr = pEvrActivate->ActivateObject(__uuidof(IMFMediaSink), reinterpret_cast<void**>(&pEvrSink)));
				IF_FAILED_THROW(hr = pEvrSink->GetStreamSinkByIndex(0, &pStreamSink1));
				IF_FAILED_THROW(hr = pEvrSink->AddStreamSink(1, NULL, &pStreamSink2));

				IF_FAILED_THROW(hr = BuildTopology(pTopology, pSourcePD, pSource, pStreamSink1, pStreamSink2));

				*ppTopology = pTopology;
				(*ppTopology)->AddRef();
		}
		catch(HRESULT) {}

		SAFE_RELEASE(pStreamSink2);
		SAFE_RELEASE(pStreamSink1);
		SAFE_RELEASE(pEvrSink);
		//SAFE_RELEASE(pVideoRenderer);
		SAFE_RELEASE(pEvrActivate);
		SAFE_RELEASE(pTopology);
		SAFE_RELEASE(pSourcePD);

		return hr;
}

HRESULT BuildTopology(IMFTopology* pTopology, IMFPresentationDescriptor* pSourcePD, IMFMediaSource* pSource, IMFStreamSink* pStreamSink1, IMFStreamSink* pStreamSink2) {

		assert(pTopology != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSourceSD = NULL;
		IMFTopologyNode* pSourceNode = NULL;
		IMFTopologyNode* pOutputNode = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		BOOL bSelected = FALSE;
		DWORD dwStreamCount;
		GUID guidMajorType = GUID_NULL;
		BOOL bRef = TRUE;

		try {

				IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorCount(&dwStreamCount));

				for(DWORD i = 0; i < dwStreamCount; i++) {

						IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorByIndex(i, &bSelected, &pSourceSD));

						if(bSelected) {

								IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
								IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

								if(guidMajorType == MFMediaType_Video) {

										IF_FAILED_THROW(hr = CreateSourceStreamNode(pSource, pSourcePD, pSourceSD, &pSourceNode));

										if(bRef) {

												bRef = FALSE;
												IF_FAILED_THROW(hr = CreateOutputNode(pSourceSD, &pOutputNode, pStreamSink1));

												IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
												IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

												IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
										}
										else {

												IF_FAILED_THROW(hr = CreateOutputNode(pSourceSD, &pOutputNode, pStreamSink2));

												IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
												IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

												IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
										}
								}
								/*else if (guidMajorType == MFMediaType_Audio) {

								IF_FAILED_THROW(hr = CreateSourceStreamNode(pSource, pSourcePD, pSourceSD, &pSourceNode));
								IF_FAILED_THROW(hr = CreateOutputNode(pSourceSD, &pOutputNode, NULL));

								IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
								IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

								IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
								}*/
								else {

										IF_FAILED_THROW(hr = pSourcePD->DeselectStream(i));
								}

								SAFE_RELEASE(pHandler);
								SAFE_RELEASE(pOutputNode);
								SAFE_RELEASE(pSourceNode);
						}

						SAFE_RELEASE(pSourceSD);
				}
		}
		catch(HRESULT) {}

		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pOutputNode);
		SAFE_RELEASE(pSourceNode);
		SAFE_RELEASE(pSourceSD);

		return hr;
}

HRESULT CreateSourceStreamNode(IMFMediaSource* pSource, IMFPresentationDescriptor* pSourcePD, IMFStreamDescriptor* pSourceSD, IMFTopologyNode** ppNode) {

		if(!pSource || !pSourcePD || !pSourceSD || !ppNode) {
				return E_POINTER;
		}

		IMFTopologyNode* pNode = NULL;
		HRESULT hr = S_OK;

		try {

				IF_FAILED_THROW(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode));

				IF_FAILED_THROW(hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource));

				IF_FAILED_THROW(hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pSourcePD));

				IF_FAILED_THROW(hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSourceSD));

				*ppNode = pNode;
				(*ppNode)->AddRef();
		}
		catch(HRESULT) {}

		SAFE_RELEASE(pNode);

		return hr;
}

HRESULT CreateOutputNode(IMFStreamDescriptor* pSourceSD, IMFTopologyNode** ppNode, IMFStreamSink* pStreamSink) {

		IMFTopologyNode* pNode = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		IMFMediaType* pMediaType = NULL;
		IMFActivate* pActivate = NULL;

		GUID guidMajorType = GUID_NULL;
		HRESULT hr = S_OK;

		try {

				IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));

				IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

				IF_FAILED_THROW(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));

				if(MFMediaType_Video == guidMajorType) {

						IF_FAILED_THROW(hr = pHandler->GetCurrentMediaType(&pMediaType));
						IF_FAILED_THROW(hr = pMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
						IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pMediaType));

						IF_FAILED_THROW(hr = pNode->SetObject(pStreamSink));
				}
				else if(MFMediaType_Audio == guidMajorType) {

						IF_FAILED_THROW(hr = MFCreateAudioRendererActivate(&pActivate));
						IF_FAILED_THROW(hr = pNode->SetObject(pActivate));
				}
				else {

						IF_FAILED_THROW(hr = E_FAIL);
				}

				*ppNode = pNode;
				(*ppNode)->AddRef();
		}
		catch(HRESULT) {}

		SAFE_RELEASE(pNode);
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pMediaType);
		SAFE_RELEASE(pActivate);

		return hr;
}

HRESULT InitWindow(const UINT uiWidth, const UINT uiHeight) {

		WNDCLASSEX WndClassEx;

		WndClassEx.cbSize = sizeof(WNDCLASSEX);
		WndClassEx.style = CS_HREDRAW | CS_VREDRAW;
		WndClassEx.lpfnWndProc = WindowApplicationMsgProc;
		WndClassEx.cbClsExtra = 0L;
		WndClassEx.cbWndExtra = 0L;
		WndClassEx.hInstance = GetModuleHandle(NULL);
		WndClassEx.hIcon = NULL;
		WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClassEx.hbrBackground = NULL;
		WndClassEx.lpszMenuName = NULL;
		WndClassEx.lpszClassName = WINDOWAPPLICATION_CLASS;
		WndClassEx.hIconSm = NULL;

		if(!RegisterClassEx(&WndClassEx)) {
				return E_FAIL;
		}

		int iWndL = uiWidth + 8 + GetSystemMetrics(SM_CXSIZEFRAME) * 2;
		int iWndH = uiHeight + 8 + GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

		int iXWnd = (GetSystemMetrics(SM_CXSCREEN) - iWndL) / 2;
		int iYWnd = (GetSystemMetrics(SM_CYSCREEN) - iWndH) / 2;

		if((g_hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, WINDOWAPPLICATION_CLASS, WINDOWAPPLICATION_CLASS, WS_OVERLAPPEDWINDOW, iXWnd, iYWnd,
				iWndL, iWndH, GetDesktopWindow(), NULL, GetModuleHandle(NULL), NULL)) == NULL) {
				return E_FAIL;
		}

		RECT rc;
		GetClientRect(g_hWnd, &rc);

		// If failed change iWndL or/and iWndH to be TRUE
		assert(rc.right == VIDEO_WIDTH_1 && rc.bottom == VIDEO_HEIGHT_1);

		ShowWindow(g_hWnd, SW_SHOW);

		return S_OK;
}

LRESULT CALLBACK WindowApplicationMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

		if(msg == WM_PAINT) {

				ValidateRect(hWnd, NULL);
				return 0L;
		}
		else if(msg == WM_ERASEBKGND) {

				return 1L;
		}
		else if(msg == WM_CLOSE) {

				PostQuitMessage(0);
				return 0L;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
}