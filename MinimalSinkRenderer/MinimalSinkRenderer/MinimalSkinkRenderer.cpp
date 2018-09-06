//----------------------------------------------------------------------------------------------
// MinimalSkinkRenderer.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

CMinimalSkinkRenderer::CMinimalSkinkRenderer(HRESULT& hr)
	: m_nRefCount(1),
	m_bShutdown(FALSE),
	m_pStreamSkinkRenderer(NULL),
	m_pClock(NULL),
	m_dwCurrentFrame(0)
{
	TRACE_SINK((L"SinkRenderer::CTOR"));

	CStreamSkinkRenderer* pStream = NULL;
	hr = CStreamSkinkRenderer::CreateInstance(this, &pStream, hr);

	if(SUCCEEDED(hr)){

		m_pStreamSkinkRenderer = pStream;
		m_pStreamSkinkRenderer->AddRef();
	}
	else{
		m_bShutdown = TRUE;
	}

	SAFE_RELEASE(pStream);
}

CMinimalSkinkRenderer::~CMinimalSkinkRenderer(){

	TRACE_SINK((L"SinkRenderer::DTOR"));
	Shutdown();
}

HRESULT CMinimalSkinkRenderer::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_SINK((L"SinkRenderer::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CMinimalSkinkRenderer* pMFSk = new (std::nothrow)CMinimalSkinkRenderer(hr);

	IF_FAILED_RETURN(pMFSk == NULL ? E_OUTOFMEMORY : S_OK);
	IF_FAILED_RETURN(FAILED(hr) ? hr : S_OK);

	LOG_HRESULT(hr = pMFSk->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFSk);

	return hr;
}

HRESULT CMinimalSkinkRenderer::QueryInterface(REFIID riid, void** ppv){

	TRACE_SINK((L"SinkRenderer::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
		QITABENT(CMinimalSkinkRenderer, IMFMediaSink),
		QITABENT(CMinimalSkinkRenderer, IMFClockStateSink),
	{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CMinimalSkinkRenderer::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"SinkRenderer::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMinimalSkinkRenderer::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"SinkRenderer::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CMinimalSkinkRenderer::ProcessSample(IMFSample* pSample){

	TRACE_SINK((L"SinkRenderer::ProcessSample"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	// if m_dwCurrentFrame == ULONG_MAX, we should need to handle
	m_dwCurrentFrame++;

	LONGLONG llTime = 0;
	hr = pSample->GetSampleTime(&llTime);

	// Log every 2 seconds
	// With big_buck_bunny_720p_50mb.mp4 : 25 fps (25 * 2 = 50) == 2 seconds
	if(SUCCEEDED(hr) && !(m_dwCurrentFrame % 50)){

		TRACE_NO_END_LINE((L"Frame at %u : ", m_dwCurrentFrame));
		MFTimeString(llTime);
	}

	// Here we should use a thread, instead of Sleep. It is just for simplcity and demonstration.
	// 1 second = 1000 ms
	// 1000 / 25 fps = 40
	// 26 instead of 40, to be relatively in time with sound
	Sleep(26);

	return hr;
}