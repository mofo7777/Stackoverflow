//----------------------------------------------------------------------------------------------
// StreamSkinkRenderer.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

CStreamSkinkRenderer::CStreamSkinkRenderer(CMinimalSkinkRenderer* pMinimalSkinkRenderer, HRESULT& hr)
	: m_nRefCount(1),
	m_State(StreamTypeNotSet),
	m_pEventQueue(NULL),
	m_pMediaType(NULL),
	m_pMinimalSkinkRenderer(NULL)
{
	TRACE_STREAM((L"StreamRenderer::CTOR"));

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));

	m_pMinimalSkinkRenderer = pMinimalSkinkRenderer;
	m_pMinimalSkinkRenderer->AddRef();
}

CStreamSkinkRenderer::~CStreamSkinkRenderer(){

	TRACE_STREAM((L"StreamRenderer::DTOR"));

	Shutdown();
}

HRESULT CStreamSkinkRenderer::CreateInstance(CMinimalSkinkRenderer* pSink, CStreamSkinkRenderer** ppStream, HRESULT& hr){

	TRACE_SINK((L"StreamRenderer::CreateInstance"));

	IF_FAILED_RETURN(hr = (ppStream == NULL ? E_INVALIDARG : S_OK));

	CStreamSkinkRenderer* pStream = new (std::nothrow)CStreamSkinkRenderer(pSink, hr);

	IF_FAILED_RETURN(pStream == NULL ? E_OUTOFMEMORY : S_OK);

	*ppStream = pStream;
	(*ppStream)->AddRef();

	SAFE_RELEASE(pStream);

	return hr;
}

HRESULT CStreamSkinkRenderer::QueryInterface(REFIID riid, void** ppv){

	TRACE_STREAM((L"StreamRenderer::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
		QITABENT(CStreamSkinkRenderer, IMFStreamSink),
		QITABENT(CStreamSkinkRenderer, IMFMediaEventGenerator),
		QITABENT(CStreamSkinkRenderer, IMFMediaTypeHandler),
	{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CStreamSkinkRenderer::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"StreamRenderer::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CStreamSkinkRenderer::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"StreamRenderer::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CStreamSkinkRenderer::Shutdown(){

	TRACE_STREAM((L"StreamRenderer::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pEventQueue){
		LOG_HRESULT(m_pEventQueue->Shutdown());
	}

	SAFE_RELEASE(m_pMediaType);
	SAFE_RELEASE(m_pMinimalSkinkRenderer);
	SAFE_RELEASE(m_pEventQueue);

	m_State = StreamFinalized;

	return S_OK;
}

HRESULT CStreamSkinkRenderer::Start(MFTIME){

	TRACE_STREAM((L"StreamRenderer::Start"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL));

	if(m_State != StreamStarted){

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));
		m_State = StreamStarted;
	}

	return hr;
}

HRESULT CStreamSkinkRenderer::Stop(){

	TRACE_STREAM((L"StreamRenderer::Stop"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL));

	m_State = StreamStopped;

	return hr;
}

HRESULT CStreamSkinkRenderer::Pause(){

	TRACE_STREAM((L"StreamRenderer::Pause"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized || m_State == StreamStopped ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL));

	m_State = StreamPaused;

	return hr;
}

HRESULT CStreamSkinkRenderer::Restart(){

	TRACE_STREAM((L"StreamRenderer::Restart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State != StreamPaused ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL));
	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

	m_State = StreamStarted;

	return hr;
}