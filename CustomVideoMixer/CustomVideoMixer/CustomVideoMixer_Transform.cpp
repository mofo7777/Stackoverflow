//----------------------------------------------------------------------------------------------
// CustomVideoMixer_Transform.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CCustomVideoMixer::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetStreamLimits"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

	*pdwInputMinimum = 1;
	*pdwInputMaximum = 16;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return hr;
}

HRESULT CCustomVideoMixer::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetStreamCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

	*pcInputStreams = m_dwInputStreamCount;
	*pcOutputStreams = 1;

	return hr;
}

HRESULT CCustomVideoMixer::GetStreamIDs(DWORD dwInputIDArraySize, DWORD* pdwInputIDs, DWORD dwOutputIDArraySize, DWORD* pdwOutputIDs) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetStreamIDs"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputIDArraySize == 0 || dwOutputIDArraySize  == 0 ? MF_E_BUFFERTOOSMALL : S_OK));
	IF_FAILED_RETURN(hr = (pdwInputIDs == NULL || pdwOutputIDs == NULL ? E_POINTER : S_OK));

	*pdwOutputIDs = 0;

	if (m_dwInputStreamCount == 1)
		*pdwInputIDs = 0;
	else
		IF_FAILED_RETURN(hr = E_FAIL);

	return hr;
}

HRESULT CCustomVideoMixer::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetInputStreamInfo"));

	TRACE_TRANSFORM((L"dwInputStreamID = %d", dwInputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID > 1 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	pStreamInfo->dwFlags =
		MFT_INPUT_STREAM_WHOLE_SAMPLES |
		MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
		MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE |
		MFT_INPUT_STREAM_DOES_NOT_ADDREF;
	pStreamInfo->hnsMaxLatency = 0;
	pStreamInfo->cbSize = 0;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 0;

	return hr;
}

HRESULT CCustomVideoMixer::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetOutputStreamInfo"));

	TRACE_TRANSFORM((L"dwOutputStreamID = %d", dwOutputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	pStreamInfo->dwFlags =
		MFT_OUTPUT_STREAM_WHOLE_SAMPLES |
		MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
		MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE |
		MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

	pStreamInfo->cbAlignment = 0;
	pStreamInfo->cbSize = 0;

	return hr;
}

HRESULT CCustomVideoMixer::GetAttributes(IMFAttributes** ppAttributes) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetAttributes"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppAttributes == NULL ? E_POINTER : S_OK));

	*ppAttributes = this;
	(*ppAttributes)->AddRef();

	return hr;
}

HRESULT CCustomVideoMixer::GetInputStreamAttributes(DWORD dwInputStreamID, IMFAttributes** ppAttributes) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetInputStreamAttributes"));

	TRACE_TRANSFORM((L"dwInputStreamID = %d", dwInputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID > 1 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (ppAttributes == NULL ? E_POINTER : S_OK));

	*ppAttributes = this;
	(*ppAttributes)->AddRef();

	return hr;
}

HRESULT CCustomVideoMixer::GetOutputStreamAttributes(DWORD dwOutputStreamID, IMFAttributes** ppAttributes) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetOutputStreamAttributes"));

	TRACE_TRANSFORM((L"dwOutputStreamID = %d", dwOutputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (ppAttributes == NULL ? E_POINTER : S_OK));

	*ppAttributes = this;
	(*ppAttributes)->AddRef();

	return hr;
}

HRESULT CCustomVideoMixer::DeleteInputStream(DWORD dwStreamID) {

	TRACE_TRANSFORM((L"CustomVideoMixer::DeleteInputStream"));

	TRACE_TRANSFORM((L"dwStreamID = %d", dwStreamID));

	if (dwStreamID == 0)
		return MF_E_INVALIDREQUEST;
	else if (dwStreamID != 1)
		return MF_E_INVALIDSTREAMNUMBER;
	else if(m_dwInputStreamCount != 2)
		return MF_E_INVALIDREQUEST;

	//MF_E_TRANSFORM_INPUT_REMAINING

	m_dwInputStreamCount--;

	return S_OK;
}

HRESULT CCustomVideoMixer::AddInputStreams(DWORD cStreams, DWORD* adwStreamIDs) {

	TRACE_TRANSFORM((L"CustomVideoMixer::AddInputStreams"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (cStreams != 1 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (adwStreamIDs == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (*adwStreamIDs != 1 ? E_INVALIDARG : S_OK));

	if (m_dwInputStreamCount == 1)
		m_dwInputStreamCount++;
	else
		IF_FAILED_RETURN(hr = E_INVALIDARG);

	return S_OK;
}

HRESULT CCustomVideoMixer::GetInputAvailableType(DWORD /*dwInputStreamID*/, DWORD /*dwTypeIndex*/, IMFMediaType** /*ppType*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetInputAvailableType"));

	TRACE_TRANSFORM((L"dwInputStreamID = %d - dwTypeIndex = %d", dwInputStreamID, dwTypeIndex));

	return MF_E_NO_MORE_TYPES;
}

HRESULT CCustomVideoMixer::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetOutputAvailableType"));

	TRACE_TRANSFORM((L"dwOutputStreamID = %d - dwTypeIndex = %d", dwOutputStreamID, dwTypeIndex));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK));

	AutoLock lock(m_CriticSection);

	if (m_pRefInputType == NULL) {
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}
	else {
		hr = GetOutputType(ppType);
	}

	return hr;
}

HRESULT CCustomVideoMixer::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetInputType"));

	TRACE_TRANSFORM((L"dwInputStreamID = %d", dwInputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID > 1 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	TRACE_TRANSFORM((L"bReallySet = %s", bReallySet ? L"TRUE" : L"FALSE"));

	AutoLock lock(m_CriticSection);

	if (pType) {

		LogMediaType(pType);
	}
	else {

		if (dwInputStreamID == 0)
			SAFE_RELEASE(m_pRefInputType);
		else
			SAFE_RELEASE(m_pSubInputType);

		return hr;
	}

	if (bReallySet) {

		if (dwInputStreamID == 0) {

			SAFE_RELEASE(m_pRefInputType);
			m_pRefInputType = pType;
			m_pRefInputType->AddRef();
		}
		else {

			SAFE_RELEASE(m_pSubInputType);
			m_pSubInputType = pType;
			m_pSubInputType->AddRef();
		}
	}

	return hr;
}

HRESULT CCustomVideoMixer::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetOutputType"));

	TRACE_TRANSFORM((L"dwOutputStreamID = %d", dwOutputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	TRACE_TRANSFORM((L"bReallySet = %s", bReallySet ? L"TRUE" : L"FALSE"));

	AutoLock lock(m_CriticSection);

	if (pType) {

		LogMediaType(pType);
	}
	else {

		SAFE_RELEASE(m_pOutputType);
		return hr;
	}

	if (bReallySet) {

		SAFE_RELEASE(m_pOutputType);
		m_pOutputType = pType;
		m_pOutputType->AddRef();
	}

	return hr;
}

HRESULT CCustomVideoMixer::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetInputCurrentType"));

	TRACE_TRANSFORM((L"dwInputStreamID = %d", dwInputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID > 1 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	IMFMediaType* m_pInputType = dwInputStreamID == 0 ? m_pRefInputType : m_pSubInputType;

	if (!m_pInputType) {
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}
	else {

		// Todo : clone MediaType
		*ppType = m_pInputType;
		(*ppType)->AddRef();
	}

	return hr;
}

HRESULT CCustomVideoMixer::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetOutputCurrentType"));

	TRACE_TRANSFORM((L"dwOutputStreamID = %d", dwOutputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	if (!m_pOutputType) {
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}
	else {

		// Todo : clone MediaType
		*ppType = m_pOutputType;
		(*ppType)->AddRef();
	}

	return hr;
}

HRESULT CCustomVideoMixer::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetInputStatus"));

	TRACE_TRANSFORM((L"dwInputStreamID = %d", dwInputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID > 1 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	// I think we can always process
	*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;

	return hr;
}

HRESULT CCustomVideoMixer::GetOutputStatus(DWORD* pdwFlags) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetOutputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	/*if (m_bHaveOuput) {
		*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
	}
	else {
		*pdwFlags = 0;
	}*/

	return hr;
}

HRESULT CCustomVideoMixer::SetOutputBounds(LONGLONG /*hnsLowerBound*/, LONGLONG /*hnsUpperBound*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetOutputBounds"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::ProcessEvent(DWORD /*dwInputStreamID*/, IMFMediaEvent* /*pEvent */) {

	TRACE_TRANSFORM((L"CustomVideoMixer::ProcessEvent"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam) {

	TRACE_TRANSFORM((L"CustomVideoMixer::ProcessMessage : %s (Param = %d)", MFTMessageString(eMessage), ulParam));

	HRESULT hr = S_OK;

	AutoLock lock(m_CriticSection);

	switch (eMessage) {

	case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
		//case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
		hr = BeginStreaming(ulParam);
		break;

	case MFT_MESSAGE_COMMAND_FLUSH:
	case MFT_MESSAGE_NOTIFY_END_STREAMING:
	case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
		hr = Flush();
		break;

	case MFT_MESSAGE_COMMAND_DRAIN:
		m_bDraining = TRUE;
		break;

	case MFT_MESSAGE_SET_D3D_MANAGER:
		hr = SetD3DManager(reinterpret_cast<IDirect3DDeviceManager9*>(ulParam));
		// hr = MF_E_UNSUPPORTED_D3D_TYPE...
		break;
	}

	return hr;
}

HRESULT CCustomVideoMixer::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags) {

	TRACE_TRANSFORM((L"CustomVideoMixer::ProcessInput"));

	TRACE_TRANSFORM((L"dwInputStreamID = %d", dwInputStreamID));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID > 1 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	if (m_bHaveRefOuput || m_bHaveSubOuput) {
		return MF_E_NOTACCEPTING;
	}

	if (SUCCEEDED(hr = m_cDxva2Manager.ProcessInput(pSample, dwInputStreamID))) {

		if (dwInputStreamID == 0) {

			m_bHaveRefOuput = TRUE;

			LOG_HRESULT(hr = m_pMediaEventSink->Notify(EC_SAMPLE_NEEDED, 0, 0));
		}
		else {

			m_bHaveSubOuput = TRUE;

			LOG_HRESULT(hr = m_pMediaEventSink->Notify(EC_SAMPLE_NEEDED, 1, 0));
		}
	}

	return hr;
}

HRESULT CCustomVideoMixer::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus) {

	TRACE_TRANSFORM((L"CustomVideoMixer::ProcessOutput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (cOutputBufferCount != 1 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = ((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].dwStreamID != 0 ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].pSample == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	if (m_bHaveRefOuput || m_bHaveSubOuput) {

		IF_FAILED_RETURN(hr = m_cDxva2Manager.ProcessOutput(pOutputSamples[0].pSample));

		if(m_bHaveRefOuput)
			m_bHaveRefOuput = FALSE;

		if (m_bHaveSubOuput)
			m_bHaveSubOuput = FALSE;
	}
	else {

		return MF_E_TRANSFORM_NEED_MORE_INPUT;
	}

	return hr;
}