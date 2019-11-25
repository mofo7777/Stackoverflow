//----------------------------------------------------------------------------------------------
// Player.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CPlayer::CreateInstance(const HWND hVideo, const int iIndex, CPlayer** ppPlayer)
{
	if(ppPlayer == NULL)
	{
		return E_POINTER;
	}

	CComPtr<CPlayer> pPlayer;

	pPlayer.Attach(new (std::nothrow)CPlayer(hVideo, iIndex));

	if(pPlayer == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*ppPlayer = pPlayer.Detach();

	return S_OK;
}

ULONG CPlayer::AddRef(){

	LONG lRef = InterlockedIncrement(&m_lRefCount);

	TRACE_REFCOUNT((L"CPlayer::AddRef (Player %d) m_nRefCount = %d", m_iIndex, lRef));

	return lRef;
}

ULONG CPlayer::Release(){

	ULONG ulCount = InterlockedDecrement(&m_lRefCount);

	TRACE_REFCOUNT((L"CPlayer::Release (Player %d) m_nRefCount = %d", m_iIndex, ulCount));

	if(ulCount == 0) {
		delete this;
	}

	return ulCount;
}

HRESULT CPlayer::QueryInterface(REFIID riid, void** ppv){

	TRACE_PLAYER((L"CPlayer::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {QITABENT(CPlayer, IMFAsyncCallback),{0}};

	return QISearch(this, qit, riid, ppv);
}

HRESULT CPlayer::Invoke(IMFAsyncResult* pResult)
{
	HRESULT hr = S_OK;

	MediaEventType meType = MEUnknown;
	CComPtr<IMFMediaEvent> pEvent;

	std::unique_lock<std::mutex> lck(m_mtx);

	if(m_pSession == NULL)
	{
		return hr;
	}

	try
	{
		IF_FAILED_THROW(m_pSession->EndGetEvent(pResult, &pEvent));
		IF_FAILED_THROW(pEvent->GetType(&meType));

		if(meType == MESessionClosed)
		{
			m_state = Closing;
			m_cv_close.notify_all();
		}
		else
		{
			IF_FAILED_THROW(hr = m_pSession->BeginGetEvent(this, NULL));
		}

		if(m_state != Closing)
		{
			HandleEvent(pEvent);
		}
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::HandleEvent(IMFMediaEvent* pEventPtr)
{
	HRESULT hr = S_OK;
	HRESULT hrStatus = S_OK;
	MediaEventType meType = MEUnknown;
	CComPtr<IMFMediaEvent> pEvent = (IMFMediaEvent*)pEventPtr;

	try
	{
		IF_FAILED_THROW(pEvent == NULL ? E_POINTER : S_OK);
		IF_FAILED_THROW(pEvent->GetType(&meType));
		IF_FAILED_THROW(pEvent->GetStatus(&hrStatus));
		IF_FAILED_THROW(hrStatus);

		switch(meType)
		{
			case MESessionTopologyStatus:
				//TRACE((L""));
				IF_FAILED_THROW(OnTopologyStatus(pEvent));
				break;

			case MESessionStopped:
				break;

			case MESessionClosed:
				//SetEvent(m_hCloseEvent);
				break;

			case MESessionEnded:
				//SetEvent(m_hCloseEvent);
				//IF_FAILED_THROW(OnPresentationEnded(pEvent));
				break;

			case MEEndOfPresentation:
				//SetEvent(m_hCloseEvent);
				//IF_FAILED_THROW(OnPresentationEnded(pEvent));
				break;

			case MENewPresentation:
				IF_FAILED_THROW(OnNewPresentation(pEvent));
				break;

			default:
				//hr = OnSessionEvent(pEvent, meType);
				hr = S_OK;
				break;
		}
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::OnTopologyStatus(IMFMediaEvent* pEvent)
{
	HRESULT hr = S_OK;
	UINT32 status;

	try
	{
		IF_FAILED_THROW(pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status));

		if(status == MF_TOPOSTATUS_READY)
		{
			IF_FAILED_THROW(StartPlayback());
		}
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::OnPresentationEnded(IMFMediaEvent*)
{
	m_state = Stopped;

	return S_OK;
}

HRESULT CPlayer::OnNewPresentation(IMFMediaEvent*)
{
	return E_NOTIMPL;
}

HRESULT CPlayer::StartPlayback()
{
	HRESULT hr = S_OK;
	PROPVARIANT varStart;
	CComPtr<IMFVideoDisplayControl> pVideoDisplay = NULL;

	PropVariantInit(&varStart);
	//varStart.vt = VT_I8;
	varStart.vt = VT_EMPTY;

	try
	{
		assert(m_state == OpenPending);
		IF_FAILED_THROW(m_pSession->Start(&GUID_NULL, &varStart));

		m_state = Started;
		m_cv_open.notify_all();

		IF_FAILED_THROW(MFGetService(m_pSession, MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)&pVideoDisplay));

		RECT rcDest = {0, 0, 164, 117};
		IF_FAILED_THROW(pVideoDisplay->SetVideoPosition(NULL, &rcDest));
	}
	catch(HRESULT){}

	PropVariantClear(&varStart);

	return hr;
}

HRESULT CPlayer::OpenUrl(LPCWSTR wszVideoFile)
{
	HRESULT hr = S_OK;

	IF_FAILED_RETURN(m_pSession != NULL ? MF_E_ALREADY_INITIALIZED : S_OK);

	CComPtr<IMFTopology> pTopology = NULL;
	CComPtr<IMFPresentationDescriptor> pSourcePD = NULL;

	try
	{
		IF_FAILED_THROW(CreateSession());
		IF_FAILED_THROW(CreateMediaSource(wszVideoFile, &m_pSource));
		IF_FAILED_THROW(m_pSource->CreatePresentationDescriptor(&pSourcePD));
		IF_FAILED_THROW(CreatePlaybackTopology(m_pSource, pSourcePD, m_hwndVideo, &pTopology, &m_pDecoder));

		{
			std::unique_lock<std::mutex> lck(m_mtx);
			IF_FAILED_THROW(m_pSession->SetTopology(0, pTopology));
		}
	}
	catch(HRESULT){}

	if(SUCCEEDED(hr))
	{
		m_state = OpenPending;
	}
	else
	{
		m_state = Closed;
	}

	return hr;
}

HRESULT CPlayer::Shutdown()
{
	HRESULT hr = S_OK;

	std::unique_lock<std::mutex> lck(m_mtx);

	const int sessionClosingTimeout = 30;

	if(m_state == OpenPending || m_state == Ready)
	{
		if(m_cv_open.wait_for(lck, std::chrono::seconds(sessionClosingTimeout)) == std::cv_status::timeout)
		{
			TRACE((L"CloseMediaSession timeout while waiting for state Started!"));
			return E_ABORT;
		}
	}

	if(m_state == Started || m_state == Paused || m_state == Stopped)
	{
		hr = m_pSession->Close();

		assert(hr == S_OK);
		m_state = Closing;
	}

	if(m_state == Closing)
	{
		if(FAILED(hr))
		{
			TRACE((L"Unable to close media session"));
			assert(false);
		}
		else if(m_cv_close.wait_for(lck, std::chrono::seconds(sessionClosingTimeout)) == std::cv_status::timeout)
		{
			TRACE((L"Something went really wrong, we hit the session closing timeout!"));
			assert(false);
		}
		else
		{
			TRACE((L"Successfully closed media session!"));
		}
	}
	else
	{
		TRACE((L"CloseMediaSession no session!"));
	}

	if(m_pSource)
	{
		LOG_HRESULT(m_pSource->Shutdown());
		m_pSource.Release();
	}

	if(m_pDecoder)
		m_pDecoder.Release();

	if(m_pSession != NULL){

		LOG_HRESULT(m_pSession->Shutdown());

		ULONG ulMFObjects = m_pSession->Release();
		m_pSession = NULL;

		if(ulMFObjects != 0)
		{
			TRACE((L"ulMFObjects = %d (%d)", ulMFObjects, m_iIndex));
		}

		assert(ulMFObjects == 0);
	}

	m_iIndex = 0;
	m_state = Closed;

	return hr;
}

HRESULT CPlayer::CreateSession()
{
	HRESULT hr = S_OK;

	assert(m_pSession == NULL && m_state == Closed);

	CComPtr<IMFMediaSession> pSession = NULL;

	try
	{
		//std::unique_lock<std::mutex> lck(m_mtx);
		IF_FAILED_THROW(MFCreateMediaSession(NULL, &pSession));
		IF_FAILED_THROW(pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));
		m_pSession = pSession.Detach();
		m_state = Ready;
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::CreateMediaSource(LPCWSTR wszVideoFile, IMFMediaSource** ppSource)
{
	HRESULT hr = S_OK;

	MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
	CComPtr<IMFSourceResolver> pSourceResolver = NULL;
	CComPtr<IMFPluginControl> pPluginCtrl = NULL;
	CComPtr<IUnknown> pSource = NULL;

	try
	{
		IF_FAILED_THROW(MFCreateSourceResolver(&pSourceResolver));
		IF_FAILED_THROW(pSourceResolver->CreateObjectFromURL(wszVideoFile, MF_RESOLUTION_MEDIASOURCE, NULL, &ObjectType, &pSource));
		IF_FAILED_THROW(pSource->QueryInterface(IID_PPV_ARGS(ppSource)));
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::CreatePlaybackTopology(IMFMediaSource* pSource, IMFPresentationDescriptor* pPD, const HWND hVideoWnd, IMFTopology** ppTopology, IMFTransform** ppDecoder)
{
	HRESULT hr = S_OK;

	CComPtr<IMFTopology> pTopology = NULL;
	CComPtr<IMFTransform> pDecoder = NULL;
	DWORD cSourceStreams = 0;

	try
	{
		IF_FAILED_THROW(MFCreateTopology(&pTopology));
		IF_FAILED_THROW(pPD->GetStreamDescriptorCount(&cSourceStreams));

		for(DWORD i = 0; i < cSourceStreams; i++)
		{
			CComPtr<IMFTransform> pDec;
			IF_FAILED_THROW(AddBranchToPartialTopology(pTopology, pSource, pPD, i, hVideoWnd, &pDec, false));

			if(pDec)
			{
				assert(pDecoder == NULL);
				pDecoder = pDec;
			}
		}

		IF_FAILED_THROW(pTopology->SetUINT32(MF_TOPOLOGY_DXVA_MODE, 1));
		IF_FAILED_THROW(pTopology->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, 0));

		*ppTopology = pTopology.Detach();
		*ppDecoder = pDecoder.Detach();
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::AddBranchToPartialTopology(IMFTopology* pTopology, IMFMediaSource* pSource, IMFPresentationDescriptor* pPD, const DWORD iStream, const HWND hVideoWnd, IMFTransform** ppDecoder, const bool)
{
	HRESULT hr = S_OK;
	CComPtr<IMFStreamDescriptor> pSD = NULL;
	CComPtr<IMFMediaTypeHandler> pMediaType = NULL;
	GUID guidMajorType = GUID_NULL;
	CComPtr<IMFActivate> pSinkActivate = NULL;
	CComPtr<IMFTopologyNode> pOutputNode = NULL;
	CComPtr<IMFTopologyNode> pSourceNode = NULL;

	try
	{
		BOOL fSelected = FALSE;
		IF_FAILED_THROW(pPD->GetStreamDescriptorByIndex(iStream, &fSelected, &pSD));
		IF_FAILED_THROW(fSelected == FALSE ? S_FALSE : S_OK);

		IF_FAILED_THROW(pSD->GetMediaTypeHandler(&pMediaType));
		IF_FAILED_THROW(pMediaType->GetMajorType(&guidMajorType));

		if(guidMajorType == MFMediaType_Video)
		{
			IF_FAILED_THROW(MFCreateVideoRendererActivate(hVideoWnd, &pSinkActivate));
			IF_FAILED_THROW(AddOutputNode(pTopology, pSinkActivate, 0, &pOutputNode));
			IF_FAILED_THROW(AddSourceNode(pTopology, pSource, pPD, pSD, &pSourceNode));
			IF_FAILED_THROW(CreateVideoDecoderTopology(pTopology, pSourceNode, pOutputNode, ppDecoder));
		}
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::AddOutputNode(IMFTopology* pTopology, IMFActivate* pActivate, const DWORD dwId, IMFTopologyNode** ppNode)
{
	HRESULT hr = S_OK;
	CComPtr<IMFTopologyNode> pNode = NULL;

	try
	{
		IF_FAILED_THROW(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));
		IF_FAILED_THROW(pNode->SetObject(pActivate));
		IF_FAILED_THROW(pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId));
		IF_FAILED_THROW(pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
		IF_FAILED_THROW(pTopology->AddNode(pNode));
		*ppNode = pNode.Detach();
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::AddSourceNode(IMFTopology* pTopology, IMFMediaSource* pSource, IMFPresentationDescriptor* pPD, IMFStreamDescriptor* pSD, IMFTopologyNode** ppNode)
{
	HRESULT hr = S_OK;
	CComPtr<IMFTopologyNode> pNode = NULL;

	try
	{
		
		IF_FAILED_THROW(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode));
		IF_FAILED_THROW(pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource));
		IF_FAILED_THROW(pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD));
		IF_FAILED_THROW(pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD));
		//IF_FAILED_THROW(MFAllocateSerialWorkQueue(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, &dWork));
		//IF_FAILED_THROW(pNode->SetUINT32(MF_TOPONODE_WORKQUEUE_ID, dWork));
		//IF_FAILED_THROW(pNode->SetString(MF_TOPONODE_WORKQUEUE_MMCSS_CLASS, L"Playback"));
		//m_dwWQSource = dWork;
		IF_FAILED_THROW(pTopology->AddNode(pNode));
		*ppNode = pNode.Detach();
	}
	catch(HRESULT){}

	return hr;
}

DEFINE_GUID(CLSID_MicrosoftDecoder, 0x62CE7E72, 0x4C71, 0x4D20, 0xB1, 0x5D, 0x45, 0x28, 0x31, 0xA8, 0x7D, 0x9D);

HRESULT CPlayer::CreateVideoDecoderTopology(IMFTopology* pTopology, IMFTopologyNode* pSourceNode, IMFTopologyNode* pOutputNode, IMFTransform** ppDecoder)
{
	HRESULT hr = S_OK;
	CComPtr<IMFTopologyNode> pDecoderNode = NULL;

	try
	{
		IF_FAILED_THROW(AddTransformNode(pTopology, CLSID_MicrosoftDecoder, &pDecoderNode, ppDecoder));
		IF_FAILED_THROW(pSourceNode->ConnectOutput(0, pDecoderNode, 0));
		IF_FAILED_THROW(pDecoderNode->ConnectOutput(0, pOutputNode, 0));

	}
	catch(HRESULT){}

	return hr;
}

HRESULT CPlayer::AddTransformNode(IMFTopology* pTopology, const CLSID& clsid, IMFTopologyNode** ppNode, IMFTransform** ppDecoder)
{
	HRESULT hr = S_OK;
	CComPtr<IMFTopologyNode> pNode;
	CComPtr<IMFAttributes> pAttr;
	CComPtr<IMFTransform> pTransformNode;

	try
	{
		IF_FAILED_THROW(MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pNode));
		IF_FAILED_THROW(CoCreateInstance(clsid, pAttr, CLSCTX_INPROC_SERVER, IID_IMFTransform, (void **)&pTransformNode));
		IF_FAILED_THROW(pNode->SetObject(pTransformNode));
		IF_FAILED_THROW(pNode->SetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, clsid));
		IF_FAILED_THROW(pTopology->AddNode(pNode));
		*ppNode = pNode.Detach();
		*ppDecoder = pTransformNode.Detach();
	}
	catch(HRESULT){}

	return hr;
}