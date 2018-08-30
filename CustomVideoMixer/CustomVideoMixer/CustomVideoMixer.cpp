//----------------------------------------------------------------------------------------------
// CustomVideoMixer.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

CCustomVideoMixer::CCustomVideoMixer()
	: m_nRefCount(1),
	m_pMediaEventSink(NULL),
	m_pRefInputType(NULL),
	m_pSubInputType(NULL),
	m_pOutputType(NULL),
	m_bDraining(FALSE),
	m_dwInputStreamCount(1),
	m_bHaveRefOuput(FALSE),
	m_bHaveSubOuput(FALSE)
{
	TRACE_TRANSFORM((L"CustomVideoMixer::CTOR"));
}

CCustomVideoMixer::~CCustomVideoMixer() {

	TRACE_TRANSFORM((L"CustomVideoMixer::DTOR"));

	AutoLock lock(m_CriticSection);

	Flush();

	m_cDxva2Manager.ReleaseDxva2();
	SAFE_RELEASE(m_pMediaEventSink);
	SAFE_RELEASE(m_pRefInputType);
	SAFE_RELEASE(m_pSubInputType);
	SAFE_RELEASE(m_pOutputType);
}

HRESULT CCustomVideoMixer::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv) {

	TRACE_TRANSFORM((L"CustomVideoMixer::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CCustomVideoMixer* pMFT = new (std::nothrow)CCustomVideoMixer;

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

ULONG CCustomVideoMixer::AddRef() {

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CustomVideoMixer::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CCustomVideoMixer::Release() {

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CustomVideoMixer::Release m_nRefCount = %d", uCount));

	if (uCount == 0) {
		delete this;
	}

	return uCount;
}

HRESULT CCustomVideoMixer::QueryInterface(REFIID riid, void** ppv) {

	TRACE_TRANSFORM((L"CustomVideoMixer::QI : riid = %s", GetIIDString(riid)));

	// IMFQualityAdvise
	// IEVRTrustedVideoPlugin

	static const QITAB qit[] = {
		QITABENT(CCustomVideoMixer, IMFVideoDeviceID),
		QITABENT(CCustomVideoMixer, IMFGetService),
		QITABENT(CCustomVideoMixer, IMFTopologyServiceLookupClient),
		QITABENT(CCustomVideoMixer, IMFTransform),
		QITABENT(CCustomVideoMixer, IMFVideoMixerControl),
		QITABENT(CCustomVideoMixer, IMFVideoProcessor),
		QITABENT(CCustomVideoMixer, IMFAttributes),
		QITABENT(CCustomVideoMixer, IMFVideoMixerBitmap),
		QITABENT(CCustomVideoMixer, IMFVideoPositionMapper),
		{ 0 }
	};

	return QISearch(this, qit, riid, ppv);
}

HRESULT CCustomVideoMixer::GetDeviceID(IID* pDeviceID) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetDeviceID"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pDeviceID == NULL ? E_POINTER : S_OK));

	*pDeviceID = IID_IDirect3DDevice9;
	return hr;
}

HRESULT CCustomVideoMixer::GetService(REFGUID guidService, REFIID riid, LPVOID* ppvObject) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetService : guidService = %s - riid = %s", MFServiceString(guidService), GetIIDString(riid)));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppvObject == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (guidService != MR_VIDEO_MIXER_SERVICE ? MF_E_UNSUPPORTED_SERVICE : S_OK));

	if (riid == IID_IMFVideoMixerControl || riid == IID_IMFVideoProcessor || riid == IID_IMFTransform) {

		hr = QueryInterface(riid, ppvObject);
	}
	else {

		LOG_HRESULT(hr = MF_E_UNSUPPORTED_SERVICE);
	}

	return hr;
}

HRESULT CCustomVideoMixer::InitServicePointers(IMFTopologyServiceLookup* pLookup) {

	TRACE_TRANSFORM((L"CustomVideoMixer::InitServicePointers"));

	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd319606(v=vs.85).aspx
	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd406901(v=vs.85).aspx

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pLookup == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	//IF_FAILED_RETURN(hr = (IsActive() ? MF_E_INVALIDREQUEST : S_OK));

	SAFE_RELEASE(m_pMediaEventSink);

	DWORD dwObjectCount = 1;

	(void)pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&m_pMediaEventSink), &dwObjectCount);

	IF_FAILED_RETURN(hr = (m_pMediaEventSink == NULL ? E_POINTER : S_OK));

	// IMFClock* pInterface = NULL;
	// (void)pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pInterface), &dwObjectCount);
	// SAFE_RELEASE(pInterface);

	// IMFVideoPresenter* pInterface = NULL;
	// (void)pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pInterface), &dwObjectCount);
	// IF_FAILED_RETURN(hr = (pInterface == NULL ? E_POINTER : S_OK));
	// SAFE_RELEASE(pInterface);

	// IMFVideoRenderer* pInterface2 = NULL;
	// (void)pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pInterface2), &dwObjectCount);
	// IF_FAILED_RETURN(hr = (pInterface2 == NULL ? E_POINTER : S_OK));
	// SAFE_RELEASE(pInterface2);

	return hr;
}

HRESULT CCustomVideoMixer::ReleaseServicePointers() {

	TRACE_TRANSFORM((L"CustomVideoMixer::ReleaseServicePointers"));

	AutoLock lock(m_CriticSection);

	SAFE_RELEASE(m_pMediaEventSink);

	return S_OK;
}

HRESULT CCustomVideoMixer::SetD3DManager(IDirect3DDeviceManager9* pDeviceManager) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetD3DManager"));

	HRESULT hr = S_OK;

	m_cDxva2Manager.ReleaseDxva2();

	if (pDeviceManager != NULL) {

		if (m_pRefInputType != NULL && m_pOutputType != NULL)
			IF_FAILED_RETURN(hr = m_cDxva2Manager.InitDxva2(pDeviceManager, m_pOutputType, m_pRefInputType, m_pSubInputType));
	}

	return hr;
}

HRESULT CCustomVideoMixer::BeginStreaming(ULONG_PTR /*ulParam*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::BeginStreaming"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pMediaEventSink == NULL ? E_POINTER : S_OK));

	//IF_FAILED_RETURN(hr = m_pMediaEventSink->Notify(EC_SAMPLE_NEEDED, ulParam, 0));
	IF_FAILED_RETURN(hr = m_pMediaEventSink->Notify(EC_SAMPLE_NEEDED, 0, 0));
	IF_FAILED_RETURN(hr = m_pMediaEventSink->Notify(EC_SAMPLE_NEEDED, 1, 0));

	// MF_E_INVALIDSTREAMNUMBER
	// MF_E_TRANSFORM_TYPE_NOT_SET

	return hr;
}

HRESULT CCustomVideoMixer::Flush() {

	TRACE_TRANSFORM((L"CustomVideoMixer::Flush"));

	m_bDraining = FALSE;
	m_bHaveRefOuput = FALSE;
	m_bHaveSubOuput = FALSE;
	return S_OK;
}